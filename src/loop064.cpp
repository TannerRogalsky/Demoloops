#include "demoloop.h"
#include "graphics/shader.h"
#include "helpers.h"
#include <glm/gtx/rotate_vector.hpp>
#include <array>
#include <tuple>
using namespace std;
using namespace demoloop;

const uint32_t CYCLE_LENGTH = 10;

const static std::string shaderCode = R"===(
uniform mediump float uTime;

#ifdef VERTEX
uniform sampler2D _tex0_;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vec4 tex = texture2D(_tex0_, VertexTexCoord.st);
  return transform_proj * model * vec4(tex.rgb - vec3(.5, .5, .5), 1.);

  // vec4 tex = texture2D(_tex0_, VertexTexCoord.st);
  // vec4 tex2 = texture2D(_tex0_, fract(tex.rb + uTime));
  // return transform_proj * model * vec4(tex2.rgb - vec3(.5, .5, .5), 1.);

  // return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  vec4 tex = texture2D(texture, fract(tc + uTime));

  float a = .13;
  // a = 1.0;
  // return vec4(tex.rgb * 1.5, a + (1.0 - pow(sin(uTime * 5000), 2.0) * a));
  return vec4(tex.rgb * 1.5, a);
}
#endif
)===";

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
  printf("%u\n", index);

  array<uint32_t, stacks * slices * 6> indices;
  index = 0;
  for (uint32_t i = 0; i < stacks; ++i) {
    for (uint32_t j = 0; j < slices; ++j) {

      const uint32_t a = i * sliceCount + j;
      const uint32_t b = i * sliceCount + j + 1;
      const uint32_t c = (i + 1) * sliceCount + j;
      const uint32_t d = (i + 1) * sliceCount + j + 1;

      // faces one and two
      indices[index++] = c;
      indices[index++] = a;
      indices[index++] = b;

      indices[index++] = c;
      indices[index++] = d;
      indices[index++] = b;
    }
  }
  printf("%u\n", index);

  return {vertices, indices};
}

function<Vertex(float, float, uint32_t, uint32_t)> plane(float width, float height) {
  return [width, height](float u, float v, uint32_t /*stacks*/, uint32_t /*slices*/) {
    return Vertex(
      (u - 0.5) * width, (1 - v - 0.5) * height, 0,
      u * 1, v * 1,
      255, 255, 255, 255
    );
  };
}



class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(CYCLE_LENGTH, 720, 720, 0, 0, 0), shader({shaderCode, shaderCode}), offset(rand()) {
    // glDisable(GL_DEPTH_TEST);
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    noiseTexture = loadTexture("loop064/rgb-perlin-seamless-512.png");

    // planeMesh(plane(0.2, 0.2, 10, 10))
    auto [vertices, indices] = parametric<128, 128>(plane(0.2, 0.2));
    gl.bufferVertices(vertices.data(), vertices.size(), GL_STATIC_DRAW);
    gl.bufferIndices(indices.data(), indices.size(), GL_STATIC_DRAW);

    indicesCount = indices.size();

    // gl.bufferVertices(planeMesh.mVertices.data(), planeMesh.mVertices.size(), GL_STATIC_DRAW);
    // gl.bufferIndices(planeMesh.mIndices.data(), planeMesh.mIndices.size(), GL_STATIC_DRAW);

    gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
  }

  ~Loop055() {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    // const float eyeRot = 0;
    const float eyeRot = cycle_ratio * DEMOLOOP_M_PI * 2;

    const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 1), eyeRot, glm::vec3(-0.3, 1, 0));
    // const glm::vec3 eye = glm::rotate(glm::vec3(0, 4, 40), eyeRot, glm::vec3(-0.3, 1, 0));
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    shader.attach();
    {
      // float uTime = powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2) / 350 + offset / static_cast<float>(RAND_MAX);
      shader.sendFloat("uTime", 1, &cycle_ratio, 1);
    }

    gl.prepareDraw();

    // gl.drawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
    gl.drawElements(GL_LINE_STRIP, indicesCount, GL_UNSIGNED_INT, 0);

    shader.detach();
  }

private:
  Shader shader;
  size_t indicesCount;
  GLuint noiseTexture;
  const int offset;
};

int main(int, char**){
  srand(time(0)); rand();

  Loop055 test;
  test.Run();

  return 0;
}
