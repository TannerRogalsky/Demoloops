#include <iostream>
#include <cmath>
#include "demoloop_opengl.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace Demoloop;

static const uint16_t NUM_VERTS = 60;
float t = 0;
const float CYCLE_LENGTH = 8;
static const float RADIUS = 0.3;

class Loop13 : public DemoloopOpenGL {
public:
  Loop13() : DemoloopOpenGL(150, 150, 150) {
    Matrix4 perspective = Matrix4::perspective(DEMOLOOP_M_PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    gl.getProjection().copy(perspective);

    Vertex points[12];
    spherePoints(points, 0, 0, 0, RADIUS);
    sphereTriangles(vertices, points);
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

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
    const float cameraX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    // const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2);
    const float cameraY = 0;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    Matrix4 lookAt = Matrix4::lookAt({cameraX, cameraY, cameraZ}, {0, 0, 0}, {0, 1, 0});
    transform.copy(lookAt);

    gl.triangles(vertices, NUM_VERTS);

    static const float SCALE = 0.2;
    transform.scale(SCALE, SCALE, SCALE);
    const uint16_t numPoints = 50;
    for (int i = 0; i < numPoints; ++i) {
      gl.pushTransform();
      Matrix4& transform = gl.getTransform();
      const float t = i;
      const float interval_cycle_ratio = fmod(t / numPoints + cycle_ratio, 1);

      const float x = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 2) * RADIUS * 4 / SCALE;
      const float y = sin(interval_cycle_ratio * DEMOLOOP_M_PI * 2) * RADIUS * 3 / SCALE;
      // const float y = sin(t / numPoints * DEMOLOOP_M_PI * 2 * interval_cycle_ratio) * RADIUS * 3 / SCALE;
      const float z = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 6) * RADIUS * 3 / SCALE;
      transform.translate(x, y, z);
      gl.triangles(vertices, NUM_VERTS);
      gl.popTransform();
    }

    gl.popTransform();
  }

private:
  Vertex vertices[NUM_VERTS];
};

int main(int, char**){
  Loop13 test;
  test.Run();

  return 0;
}
