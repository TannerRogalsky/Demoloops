#include <iostream>
#include <algorithm>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3

float t = 0;
const float CYCLE_LENGTH = 5;

class Loop021 : public Demoloop {
public:
  Loop021() : Demoloop(150, 150, 150), RADIUS(10) {
    glDisable(GL_DEPTH_TEST);

    float phi = 0.0f;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;

    for (int i = 0; i < NUM_VERTS; i++, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 0;
    }

    Matrix4 &m = gl.getTransform();
    m.translate(width / 2, height);
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    const uint32_t arms = 4;
    const uint32_t num = 10 * arms;

    for (uint32_t i = 0; i < num; ++i) {
      const float t = i;
      const float arm_index = i % arms;
      const float i_cycle_ratio = fmod(t / num + cycle_ratio, 1);

      const float phi = i_cycle_ratio * DEMOLOOP_M_PI * 2 + arm_index / arms * DEMOLOOP_M_PI * 2;

      const float x = pow(i_cycle_ratio, 5) * cosf(phi) * width / 2;
      const float y = pow(i_cycle_ratio, 2) * -height;

      gl.pushTransform();
      Matrix4 &m = gl.getTransform();
      m.translate(x, y);
      m.rotate(-DEMOLOOP_M_PI / 2);

      setColor(hsl2rgb(t / num, 1, 0.5));
      gl.triangles(vertices, NUM_VERTS);

      gl.popTransform();
    }
  }
private:
  const float RADIUS;
  Vertex vertices[NUM_VERTS * 2];
};

int main(int, char**){
  Loop021 loop;
  loop.Run();

  return 0;
}
