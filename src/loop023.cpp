#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3


const uint32_t CYCLE_LENGTH = 5;

class Loop021 : public Demoloop {
public:
  Loop021() : Demoloop(CYCLE_LENGTH, 150, 150, 150), RADIUS(10) {
    // glDisable(GL_DEPTH_TEST);

    float phi = 0.0f;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;

    for (int i = 0; i < NUM_VERTS; i++, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 0;
    }

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height, 0});
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

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
      glm::mat4 &m = gl.getTransform();
      m = glm::translate(m, {x, y, i_cycle_ratio});
      m = glm::rotate(m, -(float)DEMOLOOP_M_PI / 2, {0, 0, 1});

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
