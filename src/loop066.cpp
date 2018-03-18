#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "math_helpers.h"
#include "graphics/image.h"
#include <array>
using namespace std;
using namespace demoloop;

const uint32_t CYCLE_LENGTH = 10;
const float radius = 5;

// Icosahedron
const float p = ( 1 + sqrt( 5 ) ) / 2;
const float t = 0;
const array<Vertex, 12> indexedVertices = {{
  {- 1,  t,  0},    {1,  t,  0},   {- 1, - t,  0},   { 1, - t,  0},
   {0, - 1,  t},    {0,  1,  t},   { 0, - 1, - t},   { 0,  1, - t},
   {t,  0, - 1},    {t,  0,  1},   {- t,  0, - 1},   {- t,  0,  1}
}};

const array<uint32_t, 60> indices = {{
   0, 11,  5,    0,  5,  1,    0,  1,  7,    0,  7, 10,    0, 10, 11,
   1,  5,  9,    5, 11,  4,   11, 10,  2,   10,  7,  6,    7,  1,  8,
   3,  9,  4,    3,  4,  2,    3,  2,  6,    3,  6,  8,    3,  8,  9,
   4,  9,  5,    2,  4, 11,    6,  2, 10,    8,  6,  7,    9,  8,  1
}};

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

template<int32_t N>
void buffer(GLuint *vbos) {
  if constexpr(N >= 0) {
    const auto vertices = subdivide<N>(indexedVertices, indices);
    const auto wireframe = getWireframeGeometry(vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[N]);
    glBufferData(GL_ARRAY_BUFFER, wireframe.size() * sizeof(Vertex), wireframe.data(), GL_STATIC_DRAW);
    buffer<N - 1>(vbos);
  }
}

const uint32_t num_details = 4;

class Loop066 : public Demoloop {
public:
  Loop066() : Demoloop(CYCLE_LENGTH, 720, 720, 150, 150, 150) {
    gl.getProjection() = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    glGenBuffers(num_details, vbos);
    buffer<num_details>(vbos);
  }

  ~Loop066() {
    glDeleteBuffers(num_details, vbos);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();


    const glm::vec3 eye = glm::vec3(2, 1, 14);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    glm::mat4 transform;
    transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0, 1, 0});
    // transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * -0.5f, {0, 1, 0});
    transform = glm::scale(transform, {radius, radius, radius});

    gl.prepareDraw(transform);

    const float modulated_cycle = pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2.0);
    const uint32_t detail = min(num_details, static_cast<uint32_t>(floor(modulated_cycle * num_details) + 1));

    setColor(0, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[detail - 1]);
    gl.useVertexAttribArrays(ATTRIBFLAG_POS);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    gl.drawArrays(GL_LINES, 0, indices.size() * pow<uint32_t>(4, detail - 1) * 2);

    const uint8_t a = fmod(modulated_cycle * num_details, 1.0) * 255;
    setColor(0, 0, 0, a);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[detail]);
    gl.useVertexAttribArrays(ATTRIBFLAG_POS);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    gl.drawArrays(GL_LINES, 0, indices.size() * pow<uint32_t>(4, detail) * 2);
  }

private:
  GLuint vbos[num_details];
};

int main(int, char**){
  Loop066 test;
  test.Run();

  return 0;
}
