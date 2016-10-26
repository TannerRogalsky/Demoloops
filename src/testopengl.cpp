#include <iostream>
#include <array>
#include <cmath>
#include <SDL.h>
#include "demoloop_opengl.h"
#include "graphics/3d_primitives.h"
#include "graphics/mesh.h"
#include "hsl.h"
using namespace std;
using namespace Demoloop;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 3;

void setTriangleColor(Triangle &t, const RGB &color) {
  t.a.r = color.r;
  t.a.g = color.g;
  t.a.b = color.b;

  t.b.r = color.r;
  t.b.g = color.g;
  t.b.b = color.b;

  t.c.r = color.r;
  t.c.g = color.g;
  t.c.b = color.b;
}

class Test4 : public DemoloopOpenGL {
public:
  Test4() : DemoloopOpenGL(150, 150, 150) {
    std::cout << glGetString(GL_VERSION) << std::endl;

    Matrix4& projection = gl.getProjection();
    Matrix4 perspective = Matrix4::perspective(PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    projection.copy(perspective);

    const float RADIUS = 1;

    mesh = cube(0, 0, 0, RADIUS);
    auto indexedVertices = mesh->getIndexedVertices();
    uint32_t count = indexedVertices.size();
    float t = 0;
    for (auto i : indexedVertices) {
      auto color = hsl2rgb(t++ / count, 1, 0.5);
      Vertex &v = mesh->mVertices[i];
      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
    }
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

    gl.pushTransform();
    Matrix4& transform = gl.getTransform();
    const float cameraX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 4;
    // const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2);
    const float cameraY = 3;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 4;
    Matrix4 lookAt = Matrix4::lookAt({cameraX, cameraY, cameraZ}, {0, 0, 0}, {0, 1, 0});
    transform.copy(lookAt);

    mesh->draw(gl);

    gl.popTransform();
  }

private:
  Mesh *mesh;
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
