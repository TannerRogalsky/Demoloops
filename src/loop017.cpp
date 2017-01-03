#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 12

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

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    setColor(255, 255, 255);
    gl.lines(vertices, NUM_VERTS);

    const float interval = DEMOLOOP_M_PI * 2 / 6;
    float apothem = cos(DEMOLOOP_M_PI / 6) * RADIUS;
    float side = 2 * apothem * tan(DEMOLOOP_M_PI / 6);

    for (int i = 0; i < 6; ++i) {
      gl.pushTransform();
      glm::mat4 &m = gl.getTransform();

      int current_vertex = fmod(floor(i + cycle_ratio * 6), 6);
      float phi = current_vertex * interval + DEMOLOOP_M_PI / 3 * 2;

      float x = side * cosf(phi) * pow(sinf(cycle_ratio * DEMOLOOP_M_PI), 2);
      float y = side * sinf(phi) * pow(sinf(cycle_ratio * DEMOLOOP_M_PI), 2);

      m = glm::translate(m, {x, y, 0});

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
