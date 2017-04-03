#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "math_helpers.h"
#include "graphics/image.h"
#include <array>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 7;

template<size_t NUM_VERTS, size_t NUM_INDICES>
array<Vertex, NUM_INDICES> toNonIndexed(const array<Vertex, NUM_VERTS> &in_verts, const array<uint32_t, NUM_INDICES> &indices) {
  array<Vertex, NUM_INDICES> out;
  for (uint32_t i = 0; i < NUM_INDICES; ++i) {
    uint32_t index = indices[i];
    out[i] = in_verts[index];
  }
  return out;
}

const float radius = 3;
// Tetrahedron
// const array<Vertex, 4> indexedVertices = {{
//   {1,  1,  1},
//   {-1, -1,  1},
//   {-1,  1, -1},
//   {1, -1, -1}
// }};

// const array<uint32_t, 12> indices = {{
//   2,  1,  0,
//   0,  3,  2,
//   1,  3,  0,
//   2,  3,  1
// }};

// Octahedron
// const array<Vertex, 6> indexedVertices = {{
//   {1, 0, 0},   {- 1, 0, 0},   { 0, 1, 0},   { 0, - 1, 0},   { 0, 0, 1},   { 0, 0, - 1}
// }};

// const array<uint32_t, 8 * 3> indices = {{
//   0, 2, 4,    0, 4, 3,    0, 3, 5,    0, 5, 2,    1, 2, 5,    1, 5, 3,    1, 3, 4,    1, 4, 2
// }};

// Dodecahedron
const float p = ( 1 + sqrt( 5 ) ) / 2;
const float r = 1 / p;
const array<Vertex, 20> indexedVertices = {{
  // (±1, ±1, ±1)
  {- 1, - 1, - 1},    {- 1, - 1,  1},
  {- 1,  1, - 1},    {- 1,  1,  1},
   {1, - 1, - 1},    { 1, - 1,  1},
   {1,  1, - 1},    { 1,  1,  1},

  // (0, ±1/φ, ±φ)
   {0, - r, - p},     {0, - r,  p},
   {0,  r, - p},     {0,  r,  p},

  // (±1/φ, ±φ, 0)
  {- r, - p,  0},    {- r,  p,  0},
   {r, - p,  0},     {r,  p,  0},

  // (±φ, 0, ±1/φ)
  {- p,  0, - r},     {p,  0, - r},
  {- p,  0,  r},     {p,  0,  r}
}};

const array<uint32_t, 108> indices = {{
   3, 11,  7,      3,  7, 15,      3, 15, 13,
   7, 19, 17,      7, 17,  6,      7,  6, 15,
  17,  4,  8,     17,  8, 10,     17, 10,  6,
   8,  0, 16,      8, 16,  2,      8,  2, 10,
   0, 12,  1,      0,  1, 18,      0, 18, 16,
   6, 10,  2,      6,  2, 13,      6, 13, 15,
   2, 16, 18,      2, 18,  3,      2,  3, 13,
  18,  1,  9,     18,  9, 11,     18, 11,  3,
   4, 14, 12,      4, 12,  0,      4,  0,  8,
  11,  9,  5,     11,  5, 19,     11, 19,  7,
  19,  5, 14,     19, 14,  4,     19,  4, 17,
   1, 12, 14,      1, 14,  5,      1,  5,  9
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

const auto vertices = subdivide<5>(indexedVertices, indices);
const auto wireframe = getWireframeGeometry(vertices);


class Loop037 : public Demoloop {
public:
  Loop037() : Demoloop(720, 720, 150, 150, 150), uvTexture("uv_texture.jpg") {
    gl.getProjection() = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
  }

  void Update(float dt) {
    t += dt;
    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;


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

    setColor(0, 0, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // gl.triangles(vertices.data(), vertices.size(), transform);
    gl.lines(wireframe.data(), wireframe.size(), transform);
  }

private:
  Image uvTexture;
};

int main(int, char**){
  Loop037 test;
  test.Run();

  return 0;
}
