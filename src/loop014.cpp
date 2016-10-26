#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

static const uint16_t NUM_VERTS = 60;
float t = 0;
const float CYCLE_LENGTH = 6;
static const float RADIUS = 0.3;

class Loop14 : public Demoloop {
public:
  Loop14() : Demoloop(150, 150, 150) {
    Matrix4 perspective = Matrix4::perspective(DEMOLOOP_M_PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    gl.getProjection().copy(perspective);

    mesh = sphere(0, 0, 0, RADIUS);
    points = mesh->getIndexedVertices();
    iota(mesh->mIndices.begin(), mesh->mIndices.end(), 0);
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    // sort(&vertices[0], &vertices[NUM_VERTS], [](Vertex a, Vertex b) {
    //     return b.z < a.z;
    // });


    const float color_cycle = pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2);
    for (int i = 0; i < NUM_VERTS; ++i) {
      Vertex &v = mesh->mVertices[i];
      // const float t = i;
      // const float interval_cycle_ratio = fmod(t / NUM_VERTS + cycle_ratio, 1);
      auto color = hsl2rgb(((v.z + RADIUS + v.x + RADIUS + v.y + RADIUS) / (RADIUS * 6) + color_cycle) / 2, 1, 0.5);

      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
      v.a = 255;
    }

    gl.pushTransform();
    Matrix4& transform = gl.getTransform();
    const float cameraX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    // const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2);
    const float cameraY = 0;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    Matrix4 lookAt = Matrix4::lookAt({cameraX, cameraY, cameraZ}, {0, 0, 0}, {0, 1, 0});
    transform.copy(lookAt);

    setColor(255, 255, 255);
    mesh->draw();

    setColor(0, 0, 0);

    Vertex lines[12 * 11 * 2];
    uint32_t index = 0;
    for (int i = 0; i < 12; ++i) {
      for (int j = 0; j < 12; ++j) {
        if (i != j) {
          lines[index++] = mesh->mVertices[*std::next(points.begin(), i)];
          lines[index++] = mesh->mVertices[*std::next(points.begin(), j)];
        }
      }
    }
    gl.lines(lines, 12 * 11 * 2);

    gl.popTransform();
  }

private:
  Mesh *mesh;
  set<uint32_t> points;
};

int main(int, char**){
  Loop14 test;
  test.Run();

  return 0;
}
