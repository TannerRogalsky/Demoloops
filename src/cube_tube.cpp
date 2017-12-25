#include "demoloop.h"
#include "graphics/shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <functional>
#include <array>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 4;

const float farPlane = 40.0;

template<size_t slices, size_t stacks>
array<Vertex, stacks * slices * 6>
parametric(std::function<Vertex(float, float, uint32_t, uint32_t)> func) {
  array<Vertex, (stacks + 1) * (slices + 1)> vertices;
  uint32_t index = 0;
  const uint32_t sliceCount = slices + 1;

  for (uint32_t i = 0; i <= stacks; ++i) {
    const float v = static_cast<float>(i) / stacks;

    for (uint32_t j = 0; j <= slices; ++j) {
      const float u = static_cast<float>(j) / slices;

      vertices[index++] = func(u, v, slices, stacks);
    }
  }

  array<Vertex, stacks * slices * 6> indices;
  index = 0;
  for (uint32_t i = 0; i < stacks; ++i) {
    for (uint32_t j = 0; j < slices; ++j) {

      const float a = i * sliceCount + j;
      const float b = i * sliceCount + j + 1;
      const float c = ( i + 1 ) * sliceCount + j + 1;
      const float d = ( i + 1 ) * sliceCount + j;

      // faces one and two
      indices[index++] = vertices[a];
      indices[index++] = vertices[b];
      indices[index++] = vertices[d];

      indices[index++] = vertices[b];
      indices[index++] = vertices[c];
      indices[index++] = vertices[d];
    }
  }

  return indices;
}

template<size_t N>
array<Vertex, N / 3 * 2> getWireframeGeometry(const array<Vertex, N> &in) {
  array<Vertex, N / 3 * 2> out;

  const uint32_t numTris = N / 3;
  for (uint32_t i = 0; i < numTris; ++i) {
    uint32_t j = 0;
    const uint32_t index = 2 * i;

    const uint32_t index1 = i * 3 + j;
    out[index + 0] = in[index1];

    const uint32_t index2 = i * 3 + (j + 1) % 3;
    out[index + 1] = in[index2];
  }

  return out;
}

template<size_t N>
array<Vertex, N> getFuckedUpWireframe(const array<Vertex, N> &in) {
  array<Vertex, N> out;

  const uint32_t numTris = N / 3;
  for (uint32_t i = 0; i < numTris; ++i) {
    const uint32_t x = 3;
    const uint32_t index = i * x;

    for (uint32_t j = 0; j < x; ++j) {
      out[index + j] = in[i * x + (j % 3)];
    }
  }

  return out;
}

const static std::string surfaceShaderCode = R"===(
uniform mediump float cycle_ratio;
#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float cr = cycle_ratio;

  vec2 fw = fwidth(tc);
  float width = max(fw.x, fw.y);
  vec2 p0 = tc - .5 * width;
  vec2 p1 = tc + .5 * width;

  #define BUMPINT(x, d) \
          (floor(x) + (1.f / (1.f - (d))) * max((x) - floor(x) - (d), 0.f))
  vec2 i = (BUMPINT(p1, 0.85) - BUMPINT(p0, 0.85)) / width;
  color.rgb *= i.x * 0.25 + i.y * 0.25;
  // color.rgb *= i.x * i.y + (1.0 - i.x) * (1.0 - i.y); // checkerboard

  return color;
}
#endif
)===";

class Loop052 : public Demoloop {
public:
  Loop052() : Demoloop(720, 720, 0, 0, 0), surfaceShader({surfaceShaderCode, surfaceShaderCode}) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, farPlane);
    gl.getProjection() = perspective;
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    // cycle_ratio = 0;

    float mx = (static_cast<float>(getMouseX()) / width - 0.5) * 2.0;
    // mx = 0;
    mx = cosf(-cycle_ratio * DEMOLOOP_M_PI * 2);
    float my = (static_cast<float>(getMouseY()) / height - 0.5) * 2.0;
    // my = 0;
    my = sinf(-cycle_ratio * DEMOLOOP_M_PI * 2 * 2);

    const float eyeMoveFactor = -0.4;
    const glm::vec3 eye = glm::vec3(mx * eyeMoveFactor, my * eyeMoveFactor, 3);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, farPlane);
    gl.getTransform() = glm::lookAt(eye, target, up);

    auto verticesFunction = [cycle_ratio, mx, my](const float u, const float v, const uint32_t slices, const uint32_t stacks) {
      float z = u * farPlane - fmod(cycle_ratio * farPlane, farPlane / slices);
      float zr = z / farPlane;

      float rotation = (v + cycle_ratio) * DEMOLOOP_M_PI * 2
                + powf(zr, 2.0) * DEMOLOOP_M_PI * 4.0 * powf(sinf(cycle_ratio * DEMOLOOP_M_PI * 2), 3);

      float x = cosf(rotation) * powf(1.0 - zr, 3) - mx * 20 * powf(zr, 3);
      float y = sinf(rotation) * powf(1.0 - zr, 3) - my * 20 * powf(zr, 3);

      const RGB c = hsl2rgb(zr + cycle_ratio, 1, 0.5);
      return Vertex(x, y, z, z + cycle_ratio * slices, v * stacks, c.r, c.g, c.b, 255);
    };

    const uint32_t SIDES = 5;
    auto vertices = parametric<50, SIDES>(verticesFunction);

    glm::mat4 transform;

    surfaceShader.attach();
    surfaceShader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);
    // setColor(0, 0, 0);
    gl.triangles(vertices.data(), vertices.size(), transform);
    // setColor(255, 255, 255);
    surfaceShader.detach();

    // auto wireframe = getWireframeGeometry(vertices);
    // glLineWidth(5.0);
    // gl.lines(wireframe.data(), wireframe.size(), transform);

    {
      auto fuckedUpVerts = getFuckedUpWireframe(parametric<5, SIDES>(verticesFunction));
      glLineWidth(5.0);
      setColor(255, 255, 255);
      gl.lines(fuckedUpVerts.data(), fuckedUpVerts.size(), transform);
    }

  }

private:
  Shader surfaceShader;
};

int main(int, char**){
  Loop052 test;
  test.Run();

  return 0;
}
