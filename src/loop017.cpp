#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

const uint32_t NUM_VERTS = 6;

const uint32_t CYCLE_LENGTH = 6;

class Loop015 : public Demoloop {
public:
  Loop015() : Demoloop(CYCLE_LENGTH, 150, 150, 150), RADIUS(height / 6) {
    glDisable(GL_DEPTH_TEST);

    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;
    for (uint32_t i = 0; i < NUM_VERTS; ++i) {
      const float phi = interval * i;

      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 1;
    }

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    setColor(255, 255, 255);
    gl.lineLoop(vertices, NUM_VERTS, glm::mat4());

    const float interval = DEMOLOOP_M_PI * 2 / 6;
    float apothem = cos(DEMOLOOP_M_PI / 6) * RADIUS;
    float side = 2 * apothem * tan(DEMOLOOP_M_PI / 6);

    for (int i = 0; i < 6; ++i) {
      int current_vertex = fmod(floor(i + cycle_ratio * 6), 6);
      float phi = current_vertex * interval + DEMOLOOP_M_PI / 3 * 2;

      float x = side * cosf(phi) * pow(sinf(cycle_ratio * DEMOLOOP_M_PI), 2);
      float y = side * sinf(phi) * pow(sinf(cycle_ratio * DEMOLOOP_M_PI), 2);

      glm::mat4 m;
      m = glm::translate(m, {x, y, 0});

      gl.lineLoop(vertices, NUM_VERTS, m);
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
