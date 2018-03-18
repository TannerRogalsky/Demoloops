#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include "math_helpers.h"
#include <array>
#include <functional>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;
const float radius = 5;

template<class T>
constexpr T pow(const T base, unsigned const exponent) {
    // (parentheses not required in next line)
    return (exponent == 0)     ? 1 :
           (exponent % 2 == 0) ? pow(base, exponent/2)*pow(base, exponent/2) :
           base * pow(base, (exponent-1)/2) * pow(base, (exponent-1)/2);
}

template<uint32_t detail>
void subdivideFace(Vertex *out, uint32_t &index, const Vertex &a, const Vertex &b, const Vertex &c) {
  const uint32_t cols = pow(2, detail);
  array<array<Vertex, cols + 1>, cols + 1> v;

  for (uint32_t i = 0; i <= cols; ++i) {
    const uint32_t rows = cols - i;

    Vertex aj = mix(a, c, static_cast<float>(i) / cols);
    Vertex bj = mix(b, c, static_cast<float>(i) / cols);

    for (uint32_t j = 0; j <= rows; ++j) {
      if (j == 0 && i == cols) {
        v[i][j] = aj;
      } else {
        v[i][j] = mix(aj, bj, static_cast<float>(j) / rows);
      }
    }
  }

  for (uint32_t i = 0; i < cols; ++i) {
    for (uint32_t j = 0; j < 2 * (cols - i) - 1; ++j) {
      const uint32_t k = floor(static_cast<float>(j) / 2.0);

      if (j % 2 == 0) {
        out[index++] = v[ i ][ k + 1 ];
        out[index++] = v[ i + 1 ][ k ];
        out[index++] = v[ i ][ k ];
      } else {
        out[index++] = v[ i ][ k + 1 ];
        out[index++] = v[ i + 1 ][ k + 1 ];
        out[index++] = v[ i + 1 ][ k ];
      }
    }
  }
}

template<
  uint32_t DETAIL = 0,
  size_t NUM_VERTS,
  size_t NUM_INDICES>
array<Vertex, NUM_INDICES * pow<uint32_t>(4, DETAIL)> subdivide(const array<Vertex, NUM_VERTS> &in, const array<uint32_t, NUM_INDICES> &indices) {
  array<Vertex, NUM_INDICES * pow<uint32_t>(4, DETAIL)> out;
  uint32_t index = 0;

  for (uint32_t i = 0; i < indices.size(); i+=3) {
    const Vertex &a = in[indices[i + 0]];
    const Vertex &b = in[indices[i + 1]];
    const Vertex &c = in[indices[i + 2]];

    subdivideFace<DETAIL>(out.data(), index, a, b, c);
  }

  return out;
}

template<size_t N>
array<Vertex, N * 2> getWireframeGeometry(const array<Vertex, N> &in) {
  array<Vertex, N * 2> out;

  const uint32_t numTris = N / 3;
  for (uint32_t i = 0; i < numTris; ++i) {
    for (uint32_t j = 0; j < 3; ++j) {
      const uint32_t index = 6 * i + j * 2;

      const uint32_t index1 = i * 3 + j;
      out[index + 0].x = in[index1].x;
      out[index + 0].y = in[index1].y;
      out[index + 0].z = in[index1].z;

      const uint32_t index2 = i * 3 + (j + 1) % 3;
      out[index + 1].x = in[index2].x;
      out[index + 1].y = in[index2].y;
      out[index + 1].z = in[index2].z;
    }
  }

  return out;
}

template<size_t V, size_t I>
struct VerticesIndices {
  array<Vertex, V> vertices;
  array<uint32_t, I> indices;
};

template<size_t slices, size_t stacks>
VerticesIndices<(slices + 1) * (stacks + 1), slices * stacks * 6> parametric(function<Vertex(uint32_t, uint32_t, float, float)> func) {
  VerticesIndices<(slices + 1) * (stacks + 1), slices * stacks * 6> data;
  const uint32_t sliceCount = slices + 1;

  uint32_t index = 0;
  for (uint32_t i = 0; i <= stacks; ++i) {
    const float v = static_cast<float>(i) / stacks;

    for (uint32_t j = 0; j <= slices; ++j) {
      const float u = static_cast<float>(j) / slices;

      data.vertices[index++] = func(j, i, u, v);
    }
  }

  index = 0;
  for (uint32_t i = 0; i < stacks; ++i) {
    for (uint32_t j = 0; j < slices; ++j) {

      const float a = i * sliceCount + j;
      const float b = i * sliceCount + j + 1;
      const float c = ( i + 1 ) * sliceCount + j + 1;
      const float d = ( i + 1 ) * sliceCount + j;

      // faces one and two
      data.indices[index++] = a;
      data.indices[index++] = b;
      data.indices[index++] = d;

      data.indices[index++] = b;
      data.indices[index++] = c;
      data.indices[index++] = d;
    }
  }

  return data;
}

const static std::string diamondShaderCode = R"===(
#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  return vec4(pow(sin(tc.x * 3.14), 2.0), tc.y, 0.5, 1.0);
}
#endif
)===";

const static std::string particleShaderCode = R"===(
#ifdef VERTEX
uniform float cycle_ratio;
attribute float particleIndex;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  float t = cycle_ratio + cycle_ratio + particleIndex;
  vec4 p = model * vertpos;
  float o = fract(sin(particleIndex) * 43758.5453);

  p.y +=  cos((t - cycle_ratio) * 3.0 * 6.28 + o * 6.28 * 0.2) *
          cos((t + cycle_ratio) * 6.28) * 10.0;
  p.x += cos(t * 6.28) * 20.0;
  p.z += sin(t * 6.28) * 20.0;
  p.xz *= 1.0 + o / 5.0;
  return transform_proj * p;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  return color;
}
#endif
)===";

const auto vi = parametric<4, 3>([](const uint32_t /*ix*/, const uint32_t iy, const float u, const float v) {
  const float height = radius * 2;
  if (iy == 0 || iy == 3) {
    return Vertex(0, height * 0.75 * (v - 0.5), 0, u, v);
  } else {
    float x = radius * 0.5 * cosf(u * DEMOLOOP_M_PI * 2);
    float y = height * (v - 0.5);
    float z = radius * 0.5 * sinf(u * DEMOLOOP_M_PI * 2);
    return Vertex(x, y, z, u, v);
  }
});
const auto vertices = subdivide<0>(vi.vertices, vi.indices);
const auto wireframe = getWireframeGeometry(vertices);

const uint32_t num_particles = 500;

class Loop066 : public Demoloop {
public:
  Loop066() : Demoloop(CYCLE_LENGTH, 720, 720, 0, 0, 0),
              diamondShader({diamondShaderCode, diamondShaderCode}),
              particleShader({particleShaderCode, particleShaderCode}),
              particleIndicesLocation(particleShader.getAttribLocation("particleIndex")) {
    gl.getProjection() = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    array<float, num_particles> particleIndices;
    for (uint32_t i = 0; i < num_particles; ++i) {
      particleIndices[i] = static_cast<float>(i) / num_particles;
    }
    glGenBuffers(1, &particlesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, particlesBuffer);
    glBufferData(GL_ARRAY_BUFFER, num_particles * sizeof(float), particleIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(particleIndicesLocation, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
    glVertexAttribDivisor(particleIndicesLocation, 1);
  }

  ~Loop066() {
    glDeleteBuffers(1, &particlesBuffer);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();


    const glm::vec3 eye = glm::vec3(2, 1, 14 * 5);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    glm::mat4 transform;
    transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0, 1, 0});
    // transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * -0.5f, {0, 1, 0});
    transform = glm::scale(transform, {radius, radius, radius});

    setColor(0, 0, 0);
    diamondShader.attach();
    gl.triangles(vertices.data(), vertices.size(), transform);
    diamondShader.detach();

    transform = glm::scale(transform, {1.08, 1.08, 1.08});
    setColor(255, 255, 255, 100);
    gl.lines(wireframe.data(), wireframe.size(), transform);

    // setColor(255, 255, 255);
    setColor(10, 150, 255, 150);
    particleShader.attach();
    particleShader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);
    transform = glm::mat4();
    transform = glm::scale(transform, {0.1, 0.1, 0.1});
    gl.prepareDraw(transform);

    gl.bufferVertices(vertices.data(), vertices.size());
    gl.useVertexAttribArrays(ATTRIBFLAG_POS | (1u << particleIndicesLocation));
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    gl.drawArraysInstanced(GL_TRIANGLES, 0, vertices.size(), num_particles);
    particleShader.detach();
  }

private:
  Shader diamondShader, particleShader;
  GLuint particlesBuffer;
  const uint32_t particleIndicesLocation;
};

int main(int, char**){
  Loop066 test;
  test.Run();

  return 0;
}
