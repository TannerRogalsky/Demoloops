#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "math_helpers.h"
#include <array>
#include <algorithm>
#include <functional>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

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

void subdivideFace(Vertex *out, uint32_t &index, const Vertex &a, const Vertex &b, const Vertex &c, uint32_t detail) {
  const uint32_t cols = pow(2, detail);
  vector<vector<Vertex>> v;
  v.resize(cols + 1);

  for (uint32_t i = 0; i <= cols; ++i) {
    const uint32_t rows = cols - i;

    v[i].resize(rows + 1);

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

template<class T>
constexpr T pow(const T base, unsigned const exponent) {
    // (parentheses not required in next line)
    return (exponent == 0)     ? 1 :
           (exponent % 2 == 0) ? pow(base, exponent/2)*pow(base, exponent/2) :
           base * pow(base, (exponent-1)/2) * pow(base, (exponent-1)/2);
}

template<
  uint32_t DETAIL = 0,
  size_t NUM_VERTS,
  size_t NUM_INDICES,
  typename = typename std::enable_if<DETAIL >= 0>::type>
array<Vertex, NUM_INDICES * pow<uint32_t>(4, DETAIL)> subdivide(const array<Vertex, NUM_VERTS> &in, const array<uint32_t, NUM_INDICES> &indices) {
  array<Vertex, NUM_INDICES * pow<uint32_t>(4, DETAIL)> out;

  uint32_t index = 0;
  for (uint32_t i = 0; i < indices.size(); i+=3) {
    const Vertex &a = in[indices[i + 0]];
    const Vertex &b = in[indices[i + 1]];
    const Vertex &c = in[indices[i + 2]];

    subdivideFace(out.data(), index, a, b, c, DETAIL);
  }

  return out;
}

const auto vertices = subdivide<4>(indexedVertices, indices);

class Loop037 : public Demoloop {
public:
  Loop037() : Demoloop(720, 720, 150, 150, 150) {
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
    transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {1, 1, 0});
    transform = glm::scale(transform, {radius, radius, radius});

    setColor(0, 0, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    gl.triangles(vertices.data(), vertices.size(), transform);
  }

private:
};

int main(int, char**){
  Loop037 test;
  test.Run();

  return 0;
}
