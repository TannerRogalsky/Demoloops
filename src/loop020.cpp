#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "demoloop.h"
#include "common/quaternion.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 6;

class Loop20 : public Demoloop {
public:
  Loop20() : Demoloop(150, 150, 150) {
    Matrix4 perspective = Matrix4::perspective(DEMOLOOP_M_PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    gl.getProjection().copy(perspective);

    mesh = sphere(0, 0, 0, 0.5);
    points = mesh->getIndexedVertices();
    float t = 0;
    for (auto i : points) {
      auto color = hsl2rgb(t++ / points.size(), 1, 0.5);
      Vertex &v = mesh->mVertices[i];
      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
    }

    uint32_t index = 0;
    for (int i = 0; i < 12; ++i) {
      for (int j = 0; j < 12; ++j) {
        if (i != j) {
          lines[index++] = mesh->mVertices[*std::next(points.begin(), i)];
          lines[index++] = mesh->mVertices[*std::next(points.begin(), j)];
        }
      }
    }
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    gl.pushTransform();
    Matrix4& transform = gl.getTransform();
    const Vector3 camera = {
      // 0,//-sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3,
      // -sinf(cycle_ratio * DEMOLOOP_M_PI * 2) * 3,
      // cosf(cycle_ratio * DEMOLOOP_M_PI * 2) * 3,
      0, 0, 3
    };
    const float orientation = copysign(1, camera.z);
    const float lookX = 0;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float lookY = 0;//cosf(cycle_ratio * DEMOLOOP_M_PI * 2) * 1;//cos(cycle_ratio * DEMOLOOP_M_PI * 2);
    const float lookZ = 0;//sinf(cycle_ratio * DEMOLOOP_M_PI * 2) * 1;
    Matrix4 lookAt = Matrix4::lookAt(camera, {lookX, lookY, lookZ}, {0, orientation, 0});

    Quaternion q(0, 0, 3, pow(sinf(cycle_ratio * DEMOLOOP_M_PI), 2) * 3);
    lookAt *= q.matrix();

    transform.copy(lookAt);

    setColor(255, 255, 255);
    mesh->draw();

    setColor(0, 0, 0);
    transform.translate(0, 0, 0.001);
    gl.lines(lines, 12 * 11 * 2);

    gl.popTransform();
  }

private:
  Mesh *mesh;
  set<uint32_t> points;
  Vertex lines[12 * 11 * 2];
};

int main(int, char**){
  Loop20 test;
  test.Run();

  return 0;
}
