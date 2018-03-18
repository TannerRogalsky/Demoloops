#include "demoloop.h"
#include "graphics/shader.h"
#include <glm/gtx/rotate_vector.hpp>
#include <functional>
#include <array>
#include <tuple>
using namespace std;
using namespace demoloop;

template<size_t slices, size_t stacks>
tuple<array<Vertex, (stacks + 1) * (slices + 1)>, array<uint32_t, stacks * slices * 6>>
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

  array<uint32_t, stacks * slices * 6> indices;
  index = 0;
  for (uint32_t i = 0; i < stacks; ++i) {
    for (uint32_t j = 0; j < slices; ++j) {

      const uint32_t a = i * sliceCount + j;
      const uint32_t b = i * sliceCount + j + 1;
      const uint32_t c = (i + 1) * sliceCount + j + 1;
      const uint32_t d = (i + 1) * sliceCount + j;

      // faces one and two
      indices[index++] = d;
      indices[index++] = a;
      indices[index++] = b;

      indices[index++] = b;
      indices[index++] = c;
      indices[index++] = d;
    }
  }

  return {vertices, indices};
}

const static std::string surfaceShaderCode = R"===(
#ifdef GL_ES
#extension GL_OES_standard_derivatives : require
#endif

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  vec2 fw = fwidth(tc);
  float width = max(fw.x, fw.y);
  vec2 p0 = tc - .5 * width;
  vec2 p1 = tc + .5 * width;

  #define BUMPINT(x, d) (floor(x) + (1.0 / (1.0 - (d))) * max((x) - floor(x) - (d), 0.0))
  vec2 i = (BUMPINT(p1, 0.95) - BUMPINT(p0, 0.95)) / width;
  color.rgb *= i.x * 0.25 + i.y * 0.25;

  return color;
}
#endif
)===";

const uint32_t Z_DUPS = 4;
const uint32_t SLICES = 25 * Z_DUPS;
const uint32_t STACKS = 15;

const float CYCLE_LENGTH = 4;
const float FAR_PLANE = 400.0 * Z_DUPS;

class Loop072 : public Demoloop {
public:
  Loop072() : Demoloop(CYCLE_LENGTH, 720, 720, 26, 4, 53),
              surfaceShader({surfaceShaderCode, surfaceShaderCode}) {}

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const glm::vec3 eye = glm::vec3(0, 3, cycle_ratio * FAR_PLANE / Z_DUPS);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, FAR_PLANE);

    GL::TempTransform tempTransform(gl);
    tempTransform.get() = glm::lookAt(eye, target, up);

    GL::TempProjection tempProjection(gl);
    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    tempProjection.get() = glm::perspective(static_cast<float>(DEMOLOOP_M_PI / 4.0), aspectRatio, 0.1f, FAR_PLANE);

    auto verticesFunction = [=](const float u, const float v, const uint32_t slices, const uint32_t stacks) {
      float z = u * FAR_PLANE;
      float zr = fmod(u * Z_DUPS, 1);

      const float w = 100;
      float x = w * (v - 0.5);
      float y = 0;
      if (abs(x) > (w / stacks)) {
        const float c = zr * DEMOLOOP_M_PI * 4;
        // https://www.desmos.com/calculator/n8suedi7ho
        y -= powf(
          sinf(c) * 0.5 +
          sinf(c + DEMOLOOP_M_PI / 4) * 0.25 +
          sinf(c * 2 + DEMOLOOP_M_PI * 0.4) * 0.5 +
          sinf(c * 6) * 0.15 +
          sinf(x * DEMOLOOP_M_PI) * 0.15
        , 2) * 10;
      }

      const RGB c = hsl2rgb(fmod(zr + cycle_ratio, 1), 1, 0.5);
      const uint8_t a = 255 * (1.0 - powf(u, 16));

      glm::vec3 p = glm::vec3(x, y, z);
      p = glm::rotate(p, static_cast<float>((v - 0.5) * -DEMOLOOP_M_PI * 0.5), {0, 0, 1});

      return Vertex(p.x, p.y, p.z, u * slices, v * stacks, c.r, c.g, c.b, a);
    };

    glm::mat4 transform;
    transform = glm::rotate(transform, static_cast<float>(DEMOLOOP_M_PI), {0, 0, 1});

    auto [vertices, indices] = parametric<SLICES, STACKS>(verticesFunction);
    surfaceShader.attach();
    gl.triangles(vertices.data(), vertices.size(), indices.data(), indices.size(), transform);
    surfaceShader.detach();
  }

private:
  Shader surfaceShader;
};

int main(int, char**){
  Loop072 test;
  test.Run();

  return 0;
}
