#include <iostream>
#include "demoloop_opengl.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace Demoloop;

float t = 0;
const float CYCLE_LENGTH = 3;

class Loop10 : public DemoloopOpenGL {
public:
  Loop10() : DemoloopOpenGL(150, 150, 150) {
    Matrix4 perspective = Matrix4::perspective(DEMOLOOP_M_PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    gl.getProjection().copy(perspective);
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    static const uint16_t NUM_VERTS = 36;
    static const uint16_t RADIUS = 1;
    cube(vertices, 0, 0, 0, RADIUS);
    for (int i = 0; i < NUM_VERTS; ++i) {
      const float t = i;
      const float interval_cycle_ratio = fmod(t / NUM_VERTS + cycle_ratio, 1);
      auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);

      vertices[i].r = color.r;
      vertices[i].g = color.g;
      vertices[i].b = color.b;
      vertices[i].a = 255;
    }

    gl.pushTransform();
    Matrix4& transform = gl.getTransform();
    const float cameraX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 4;
    const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2) * 3;
    const float cameraZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    Matrix4 lookAt = Matrix4::lookAt({cameraX, cameraY, cameraZ}, {0, 0, 0}, {0, 1, 0});
    transform.copy(lookAt);

    gl.triangles(vertices, NUM_VERTS);
    gl.popTransform();
  }

private:
  Vertex vertices[36];
};

int main(int, char**){
  Loop10 test;
  test.Run();

  return 0;
}
