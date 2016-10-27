#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 6;
static const float RADIUS = 0.3;

class Loop19 : public Demoloop {
public:
  Loop19() : Demoloop(150, 150, 150) {
    Matrix4 perspective = Matrix4::perspective(DEMOLOOP_M_PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    gl.getProjection().copy(perspective);

    mesh = sphere(0, 0, 0, RADIUS);
    auto indexedVertices = mesh->getIndexedVertices();
    float t = 0;
    for (auto i : indexedVertices) {
      auto color = hsl2rgb(t++ / indexedVertices.size(), 1, 0.5);
      Vertex &v = mesh->mVertices[i];
      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
    }
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    gl.pushTransform();
    Matrix4& transform = gl.getTransform();
    const float lookX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float lookY = 0;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float lookZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraX = 0;//pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2) * 3;
    const float cameraY = 0;
    const float cameraZ = 0;
    Matrix4 lookAt = Matrix4::lookAt({lookX, lookY, lookZ}, {cameraX, cameraY, cameraZ}, {0, 1, 0});
    transform.copy(lookAt);

    // setColor(255, 255, 255);
    mesh->draw();

    const uint32_t numLines = 12 * 2 * 2;
    Vertex lines[numLines];
    uint32_t index = 0;
    for (int i = 0; i < 12; ++i) {
      const float t = i;
      const auto color = hsl2rgb(t / 12, 1, 0.5);

      lines[index].x = 0;
      lines[index].y = 0;
      lines[index].z = 0;
      lines[index].r = 0;
      lines[index].g = 0;
      lines[index].b = 0;
      index++;

      lines[index].x = 0;
      lines[index].y = sinf(i * DEMOLOOP_M_PI * 2 / 12);
      lines[index].z = cosf(i * DEMOLOOP_M_PI * 2 / 12);
      lines[index].r = color.r;
      lines[index].g = color.g;
      lines[index].b = color.b;
      index++;
    }

    for (int i = 0; i < 12; ++i) {
      lines[index].x = 0;
      lines[index].y = sinf(i * DEMOLOOP_M_PI * 2 / 12);
      lines[index].z = cosf(i * DEMOLOOP_M_PI * 2 / 12);
      index++;

      lines[index].x = 0;
      lines[index].y = sinf((i + 1) * DEMOLOOP_M_PI * 2 / 12);
      lines[index].z = cosf((i + 1) * DEMOLOOP_M_PI * 2 / 12);
      index++;
    }

    transform.rotate(cycle_ratio * DEMOLOOP_M_PI * 2);
    gl.lines(lines, numLines);

    gl.popTransform();
  }

private:
  Mesh *mesh;
};

int main(int, char**){
  Loop19 test;
  test.Run();

  return 0;
}
