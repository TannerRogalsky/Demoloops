#include <iostream>
#include <algorithm>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 60

float t = 0;
const float CYCLE_LENGTH = 6;

class Loop015 : public Demoloop {
public:
  Loop015() : Demoloop(150, 150, 150), RADIUS(height / 6) {
    glDisable(GL_DEPTH_TEST);

    float phi = 0.0f;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS * 2;

    for (int i = 0; i < NUM_VERTS - 1; i+=2, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 1;

      vertices[i + 1].x = RADIUS * cosf(phi + interval);
      vertices[i + 1].y = RADIUS * sinf(phi + interval);
      vertices[i + 1].z = 1;
    }

    Matrix4 &m = gl.getTransform();
    m.translate(width / 2, height / 2);
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    gl.lines(vertices, NUM_VERTS);

    const float interval = DEMOLOOP_M_PI * 2 / 6;

    for (int i = 0; i < 6; ++i) {
      gl.pushTransform();
      Matrix4 &m = gl.getTransform();
      m.rotate(interval * i + cycle_ratio * DEMOLOOP_M_PI * 2);
      m.translate(RADIUS, 0);

      gl.lines(vertices, NUM_VERTS);

      gl.popTransform();
    }

    for (int i = 0; i < 6; ++i) {
      gl.pushTransform();
      Matrix4 &m = gl.getTransform();
      m.rotate(interval * i + DEMOLOOP_M_PI / 6);
      m.translate(RADIUS * 2 * cosf(DEMOLOOP_M_PI / 6), 0);

      gl.lines(vertices, NUM_VERTS);

      gl.popTransform();
    }
  }
private:
  const float RADIUS;
  Vertex vertices[NUM_VERTS];
};

int main(int, char**){
  Loop015 loop;
  loop.Run();

  return 0;
}
