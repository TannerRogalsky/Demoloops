#include <iostream>
#include <cmath>
#include <SDL.h>
#include "demoloop_opengl.h"
using namespace std;
using namespace Demoloop;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 3;

class Test4 : public DemoloopOpenGL {
public:
  Test4() : DemoloopOpenGL(150, 150, 150) {
    std::cout << glGetString(GL_VERSION) << std::endl;
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    const uint16_t num_vertices = 6;
    const uint16_t RADIUS = height / 3;

    const float interval = PI * 2 / num_vertices;

    Demoloop::Vertex coords[num_vertices];
    for (int i = 0; i < num_vertices; ++i) {
      const float t = i;
      coords[i].x = cos(interval * t) * RADIUS + width / 2 + sin(cycle_ratio * PI * 2) * RADIUS / 2;
      coords[i].y = sin(interval * t) * RADIUS + height / 2 + sin(cycle_ratio * PI * 2) * RADIUS / 2;
    }

    gl.pushTransform();
    Matrix4& transform = gl.getTransform();
    transform.translate(100, 100);

    gl.polygon(coords, num_vertices);
    gl.popTransform();
  }

private:
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
