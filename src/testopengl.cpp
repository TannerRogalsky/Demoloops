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

class Test4 : public Demoloop {
public:
  Test4() : Demoloop(150, 150, 150), mesh(nullptr), canvas(100, 100) {
    std::cout << glGetString(GL_VERSION) << std::endl;

    // Matrix4& projection = gl.getProjection();
    // Matrix4 perspective = Matrix4::perspective(PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    // projection.copy(perspective);
    gl.getProjection() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

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

    // glm::mat4 modelView(1);
    // // modelView = glm::translate(modelView, {20, 20, 0});
    // canvas.draw(modelView);

    // rectangle(gl, height / 2, height / 2, 50, 50);

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    // gl.pushTransform();
    // Matrix4& transform = gl.getTransform();
    // const float cameraX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 4;
    // // const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2);
    // const float cameraY = 3;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    // const float cameraZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 4;
    // Matrix4 lookAt = Matrix4::lookAt({cameraX, cameraY, cameraZ}, {0, 0, 0}, {0, 1, 0});
    // transform.copy(lookAt);

    // const glm::vec3 rotationAxis = glm::normalize(glm::vec3(-1, sinf(rad) / 3, 0));
    // const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 3 + pow(sinf(pow(cycle_ratio, 3) * DEMOLOOP_M_PI), 2) * 10), rad, rotationAxis);
    // const glm::vec3 up = glm::rotate(glm::vec3(0, 1, 0), rad, rotationAxis);
    // const glm::vec3 target = glm::rotate(glm::vec3(0, 2, 0), rad, rotationAxis);

    const glm::vec3 eye = glm::rotate(glm::vec3(4, 0, 10), static_cast<float>(-cycle_ratio * DEMOLOOP_M_PI * 2), glm::vec3(0, 1, 0));
    // const glm::vec3 eye = glm::vec3(4, 0, 10);
    const glm::vec3 target = {0, 0, 0};
    const glm::vec3 up = {0, 1, 0};
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    mesh->draw(glm::rotate(glm::mat4(), cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(1, 1, 0)));

    // gl.popTransform();
  }

private:
  Mesh *mesh;
  Canvas canvas;
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
