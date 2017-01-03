#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3

float t = 0;
const float CYCLE_LENGTH = 5;

float inQuint(float t, float b, float c, float d) {
  return c * pow(t / d, 5) + b;
}

class Loop021 : public Demoloop {
public:
  Loop021() : Demoloop(0, 0, 0), RADIUS(10) {
    glDisable(GL_DEPTH_TEST);

    float phi = 0.0f;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;

    for (int i = 0; i < NUM_VERTS; i++, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 0;
    }

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    const uint16_t num = CYCLE_LENGTH * 4;
    const uint16_t arms = 2;
    for (int i = 0; i < num; ++i) {
      const float t = i;
      const float n = fmod(i + cycle_ratio * num, num);
      const uint16_t arm_index = i % arms;
      const float phi = n / num * DEMOLOOP_M_PI * 2 + DEMOLOOP_M_PI / 4 * arm_index;
      const float ratio = -n / num;
      const float x = cosf(-phi) * ratio * height * 0.4;
      const float y = sinf(-phi) * ratio * height * 0.4;

      gl.pushTransform();
      glm::mat4 &m = gl.getTransform();
      m = glm::translate(m, {x, y, 0});
      m = glm::rotate(m, phi, {0, 0, 1});

      setColor(hsl2rgb(t / num, 1, 0.5), inQuint(ratio, 0, 1, 1) * 255);
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
