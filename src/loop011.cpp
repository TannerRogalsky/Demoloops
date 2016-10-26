#include <iostream>
#include <numeric>
#include "demoloop_opengl.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace Demoloop;

static const uint16_t NUM_VERTS = 60;
float t = 0;
const float CYCLE_LENGTH = 3;
static const float RADIUS = 0.3;

class Loop11 : public DemoloopOpenGL {
public:
  Loop11() : DemoloopOpenGL(150, 150, 150) {
    Matrix4 perspective = Matrix4::perspective(DEMOLOOP_M_PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    gl.getProjection().copy(perspective);

    mesh = sphere(0, 0, 0, RADIUS);
    iota(mesh->mIndices.begin(), mesh->mIndices.end(), 0);
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    for (int i = 0; i < NUM_VERTS; ++i) {
      const float t = i;
      const float interval_cycle_ratio = fmod(t / NUM_VERTS + cycle_ratio, 1);
      auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);

      Vertex &v = mesh->mVertices[i];
      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
      v.a = 255;
    }

    gl.pushTransform();
    Matrix4& transform = gl.getTransform();
    const float cameraX = 0;//sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    // const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2);
    const float cameraY = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraZ = 3;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    Matrix4 lookAt = Matrix4::lookAt({cameraX, cameraY, cameraZ}, {0, 0, 0}, {0, 1, 0});
    transform.copy(lookAt);

    mesh->draw();
    transform.translate(0, sin(cycle_ratio * DEMOLOOP_M_PI * 2) * RADIUS * 4, 0);
    transform.scale(0.5, 0.5, 0.5);
    mesh->draw();

    gl.popTransform();
  }

private:
  Mesh *mesh;
};

int main(int, char**){
  Loop11 test;
  test.Run();

  return 0;
}
