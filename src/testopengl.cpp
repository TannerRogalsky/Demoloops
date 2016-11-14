#include <iostream>
#include <array>
#include <cmath>
#include <SDL.h>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/2d_primitives.h"
#include "graphics/mesh.h"
#include "graphics/canvas.h"
#include "hsl.h"
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

const static std::string shaderCode = R"===(
#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords) {
  vec4 texturecolor = Texel(texture, texture_coords);
  return texturecolor * color;
}
#endif
)===";

class Test4 : public Demoloop {
public:
  Test4() : Demoloop(150, 150, 150), mesh(nullptr), canvas(100, 100), shader({shaderCode, shaderCode}) {
    std::cout << glGetString(GL_VERSION) << std::endl;
    gl.getProjection() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    glEnable(GL_CULL_FACE);

    const float RADIUS = 1;
    mesh = cube(0, 0, 0, RADIUS);
    // auto indexedVertices = mesh->getIndexedVertices();
    // uint32_t count = indexedVertices.size();
    // float t = 0;
    // for (auto i : indexedVertices) {
    //   auto color = hsl2rgb(t++ / count, 1, 0.5);
    //   Vertex &v = mesh->mVertices[i];
    //   v.r = color.r;
    //   v.g = color.g;
    //   v.b = color.b;
    // }

    setCanvas(&canvas);
    setColor(255, 0, 0);
    rectangle(gl, 0, 0, 50, 50);
    setColor(0, 255, 0);
    rectangle(gl, 0, 50, 50, 50);
    setColor(0, 0, 255);
    rectangle(gl, 50, 0, 50, 50);
    setColor(255, 255, 255);
    rectangle(gl, 50, 50, 50, 50);
    setCanvas();

    mesh->setTexture(&canvas);
  }

  ~Test4() {
    if (mesh) {
      delete mesh;
    }
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    const glm::vec3 eye = glm::rotate(glm::vec3(4, 0, 10), static_cast<float>(-cycle_ratio * DEMOLOOP_M_PI * 2), glm::vec3(0, 1, 0));
    // const glm::vec3 eye = glm::vec3(4, 0, 10);
    const glm::vec3 target = {0, 0, 0};
    const glm::vec3 up = {0, 1, 0};
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    glm::mat4 m = glm::rotate(glm::mat4(), cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(1, 1, 0));

    shader.attach();
    mesh->draw(m);
    mesh->draw(glm::translate(m, {1, 0, 0.5}) * glm::rotate(m, (float) DEMOLOOP_M_PI / 2, glm::vec3(1, 0, 1)));
    shader.detach();

    // gl.popTransform();
  }

private:
  Mesh *mesh;
  Canvas canvas;
  Shader shader;
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
