#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "math_helpers.h"
#include "graphics/image.h"
#include <array>
using namespace std;
using namespace demoloop;


const uint32_t CYCLE_LENGTH = 7;

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
const array<Vertex, 4> indexedVertices = {{
  {1,  1,  1},
  {-1, -1,  1},
  {-1,  1, -1},
  {1, -1, -1}
}};

const array<uint32_t, 12> indices = {{
  2,  1,  0,
  0,  3,  2,
  1,  3,  0,
  2,  3,  1
}};

// Octahedron
// const array<Vertex, 6> indexedVertices = {{
//   {1, 0, 0},   {- 1, 0, 0},   { 0, 1, 0},   { 0, - 1, 0},   { 0, 0, 1},   { 0, 0, - 1}
// }};

// const array<uint32_t, 8 * 3> indices = {{
//   0, 2, 4,    0, 4, 3,    0, 3, 5,    0, 5, 2,    1, 2, 5,    1, 5, 3,    1, 3, 4,    1, 4, 2
// }};

// Dodecahedron
// const float p = ( 1 + sqrt( 5 ) ) / 2;
// const float r = 1 / p;
// const array<Vertex, 20> indexedVertices = {{
//   // (±1, ±1, ±1)
//   {- 1, - 1, - 1},    {- 1, - 1,  1},
//   {- 1,  1, - 1},    {- 1,  1,  1},
//    {1, - 1, - 1},    { 1, - 1,  1},
//    {1,  1, - 1},    { 1,  1,  1},

//   // (0, ±1/φ, ±φ)
//    {0, - r, - p},     {0, - r,  p},
//    {0,  r, - p},     {0,  r,  p},

//   // (±1/φ, ±φ, 0)
//   {- r, - p,  0},    {- r,  p,  0},
//    {r, - p,  0},     {r,  p,  0},

//   // (±φ, 0, ±1/φ)
//   {- p,  0, - r},     {p,  0, - r},
//   {- p,  0,  r},     {p,  0,  r}
// }};

// const array<uint32_t, 108> indices = {{
//    3, 11,  7,      3,  7, 15,      3, 15, 13,
//    7, 19, 17,      7, 17,  6,      7,  6, 15,
//   17,  4,  8,     17,  8, 10,     17, 10,  6,
//    8,  0, 16,      8, 16,  2,      8,  2, 10,
//    0, 12,  1,      0,  1, 18,      0, 18, 16,
//    6, 10,  2,      6,  2, 13,      6, 13, 15,
//    2, 16, 18,      2, 18,  3,      2,  3, 13,
//   18,  1,  9,     18,  9, 11,     18, 11,  3,
//    4, 14, 12,      4, 12,  0,      4,  0,  8,
//   11,  9,  5,     11,  5, 19,     11, 19,  7,
//   19,  5, 14,     19, 14,  4,     19,  4, 17,
//    1, 12, 14,      1, 14,  5,      1,  5,  9
// }};

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

template<size_t N>
void correctSeam(array<Vertex, N> &v) {
  for (uint32_t i = 0; i < N; i+=3) {
    const float u0 = v[i + 0].s;
    const float u1 = v[i + 1].s;
    const float u2 = v[i + 2].s;

    const float maxU = max({u0, u1, u2});
    const float minU = min({u0, u1, u2});

    if (maxU > 0.9 && minU < 0.1) {
      if (u0 < 0.2) v[i + 0].s += 1;
      if (u1 < 0.2) v[i + 1].s += 1;
      if (u2 < 0.2) v[i + 2].s += 1;
    }
  }
}

float azimuth(const Vertex &v) {
  return atan2(v.z, -v.x);
}

float inclination(const Vertex &v) {
  return atan2(-v.y, sqrt(v.x * v.x + v.z * v.z));
}

void correctUV(Vertex &v, const glm::vec2 &uv, const float azimuth) {
  if ((azimuth < 0) && (uv.x == 1)) {
    v.s =  uv.x - 1;
  }

  if ((v.x == 0) && (v.z == 0)) {
    v.s = azimuth / 2.0 / DEMOLOOP_M_PI + 0.5;
  }
}

template<size_t N>
void correctUVs(array<Vertex, N> &v) {
  glm::vec3 a, b, c, centroid;
  glm::vec2 uvA, uvB, uvC;

  for (uint32_t i = 0; i < N; i+=3) {
    memcpy(&a, &v[i + 0].x, sizeof(glm::vec3));
    memcpy(&b, &v[i + 1].x, sizeof(glm::vec3));
    memcpy(&c, &v[i + 2].x, sizeof(glm::vec3));

    memcpy(&uvA, &v[i + 0].s, sizeof(glm::vec2));
    memcpy(&uvB, &v[i + 1].s, sizeof(glm::vec2));
    memcpy(&uvC, &v[i + 2].s, sizeof(glm::vec2));

    centroid = (a + b + c) / 3.0f;

    const float azimuth = atan2(centroid.z, -centroid.x);

    correctUV(v[i + 0], uvA, azimuth);
    correctUV(v[i + 1], uvB, azimuth);
    correctUV(v[i + 2], uvC, azimuth);
  }
}

template<size_t N>
array<Vertex, N> generateUVs(array<Vertex, N> &vertices) {
  for (Vertex &v : vertices) {
    v.s = azimuth(v) / 2.0 / DEMOLOOP_M_PI + 0.5;
    v.t = inclination(v) / DEMOLOOP_M_PI + 0.5;
  }

  // correctSeam(vertices);
  // correctUVs(vertices);

  return vertices;
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

  generateUVs(out);

  return out;
}

const auto vertices = subdivide<0>(indexedVertices, indices);

class Loop037 : public Demoloop {
public:
  Loop037() : Demoloop(CYCLE_LENGTH, 480, 480, 150, 150, 150), uvTexture("uv_texture.jpg") {
    gl.getProjection() = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    // Vertex v = {100, 200, 300, 1, 1, 4, 5, 6, 7};
    // glm::vec3 pos;
    // glm::vec2 uvs;
    // printf("%f\n", uvs.x);
    // memcpy(&pos, &v, sizeof(glm::vec3));
    // memcpy(&uvs, &v.s, sizeof(glm::vec2));
    // printf("%f\n", uvs.x);

    // glm::vec3 &a = *((glm::vec3 *) &v);
    // a.x = 1.0;
    // printf("%f\n", v.x);
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

    // setColor(0, 0, 0);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    gl.triangles(vertices.data(), vertices.size(), transform);
  }

private:
  Image uvTexture;
};

int main(int, char**){
  Loop037 test;
  test.Run();

  return 0;
}
