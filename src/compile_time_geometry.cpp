#include "common/math.h"
#include <array>
#include <cmath>
using namespace std;
using namespace demoloop;

const float radius = 5;

template <
  typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
> constexpr T mix(T const &a, T const &b, const float &ratio) {
  return a * (1.0f - ratio) + b * ratio;
}

constexpr Vertex mix(const Vertex &a, const Vertex &b, const float &ratio) {
  return {
    mix(a.x, b.x, ratio),
    mix(a.y, b.y, ratio),
    mix(a.z, b.z, ratio),

    mix(a.s, b.s, ratio),
    mix(a.t, b.t, ratio),

    mix(a.r, b.r, ratio),
    mix(a.g, b.g, ratio),
    mix(a.b, b.b, ratio),
    mix(a.a, b.a, ratio)
  };
}

template<class T>
constexpr T pow(const T base, unsigned const exponent) {
    // (parentheses not required in next line)
    return (exponent == 0)     ? 1 :
           (exponent % 2 == 0) ? pow(base, exponent/2)*pow(base, exponent/2) :
           base * pow(base, (exponent-1)/2) * pow(base, (exponent-1)/2);
}

template<uint32_t detail>
constexpr void subdivideFace(Vertex *out, uint32_t &index, const Vertex &a, const Vertex &b, const Vertex &c) {
  const uint32_t cols = pow(2, detail);
  array<array<Vertex, cols + 1>, cols + 1> v;

  for (uint32_t i = 0; i <= cols; ++i) {
    const uint32_t rows = cols - i;

    const Vertex aj = mix(a, c, static_cast<float>(i) / cols);
    const Vertex bj = mix(b, c, static_cast<float>(i) / cols);

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
constexpr array<Vertex, NUM_INDICES * pow<uint32_t>(4, DETAIL)> subdivide(const array<Vertex, NUM_VERTS> &in, const array<uint32_t, NUM_INDICES> &indices) {
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
constexpr array<Vertex, N * 2> getWireframeGeometry(const array<Vertex, N> &in) {
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
  constexpr VerticesIndices<V, I>() {}
  array<Vertex, V> vertices;
  array<uint32_t, I> indices;
};

template<size_t slices, size_t stacks>
constexpr VerticesIndices<(slices + 1) * (stacks + 1), slices * stacks * 6> parametric() {
  VerticesIndices<(slices + 1) * (stacks + 1), slices * stacks * 6> data;
  const uint32_t sliceCount = slices + 1;

  uint32_t index = 0;
  for (uint32_t i = 0; i <= stacks; ++i) {
    const float v = static_cast<float>(i) / stacks;

    for (uint32_t j = 0; j <= slices; ++j) {
      const float u = static_cast<float>(j) / slices;

      const float height = radius * 2;
      if (i == 0 || i == 3) {
        data.vertices[index++] = {0, height * 0.75f * (v - 0.5f), 0, u, v};
      } else {
        float x = radius * 0.5 * cosf(u * DEMOLOOP_M_PI * 2);
        float y = height * (v - 0.5);
        float z = radius * 0.5 * sinf(u * DEMOLOOP_M_PI * 2);
        data.vertices[index++] = {x, y, z, u, v};
      }
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

int main(int, char**){
  constexpr auto vi = parametric<4, 3>();
  printf("vertices: %zu, indices: %zu\n", vi.vertices.size(), vi.indices.size());
  // constexpr auto vertices = subdivide<0>(vi.vertices, vi.indices);
  // printf("non-indexed: %zu\n", vertices.size());
  // constexpr auto wireframe = getWireframeGeometry(vertices);
  // printf("wireframe: %zu\n", wireframe.size());

  return 0;
}
