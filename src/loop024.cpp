
#include <algorithm>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3

float t = 0;
const float CYCLE_LENGTH = 10;

class Loop024 : public Demoloop {
public:
  Loop024() : Demoloop(150, 150, 150), RADIUS(10) {
    // glDisable(GL_DEPTH_TEST);

    // cout << glGetString(GL_EXTENSIONS) << endl;

    float phi = 0.0f;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;

    for (int i = 0; i < NUM_VERTS; i++, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 1;
    }

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    const uint32_t arms = 7;
    const uint32_t num = 10 * arms;

    gl.pushTransform();
    glm::mat4 &m = gl.getTransform();
    m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, {0, 0, 1});

    for (uint32_t i = 0; i < num; ++i) {
      const float t = i;
      const float i_cycle_ratio = fmod(t / num + cycle_ratio, 1);

      const float phi = i_cycle_ratio * DEMOLOOP_M_PI * 2 + DEMOLOOP_M_PI * 2;

      const float x = pow(i_cycle_ratio, 2) * cosf(phi) * width / 2;
      const float y = pow(i_cycle_ratio, 2) * -height / 2;

      gl.pushTransform();
      glm::mat4 &m = gl.getTransform();
      m = glm::rotate(m, t * ((float)DEMOLOOP_M_PI * 2 / arms), {0, 0, 1});
      m = glm::translate(m, {x, y, i_cycle_ratio});
      m = glm::rotate(m, -(float)DEMOLOOP_M_PI / 2, {0, 0, 1});

      setColor(hsl2rgb(t / num, 1, 0.5), 255 - pow(i_cycle_ratio, 3) * 255);
      gl.triangles(vertices, NUM_VERTS);

      gl.popTransform();
    }

    gl.popTransform();
  }
private:
  const float RADIUS;
  Vertex vertices[NUM_VERTS * 2];
};

int main(int, char**){
  Loop024 loop;
  loop.Run();

  return 0;
}
