#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "math_helpers.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3

float t = 0;
const float CYCLE_LENGTH = 6;
const float rotation_offset = rotationOffset(NUM_VERTS);

class Loop021 : public Demoloop {
public:
  Loop021() : Demoloop(CYCLE_LENGTH, 150, 150, 150), RADIUS(height / 6) {
    glDisable(GL_DEPTH_TEST);

    float phi = 0.0f;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;

    for (int i = 0; i < (NUM_VERTS * 2) - 1; i+=2, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi - rotation_offset);
      vertices[i].y = RADIUS * sinf(phi - rotation_offset);
      vertices[i].z = 0;

      vertices[i + 1].x = RADIUS * cosf(phi - rotation_offset + interval);
      vertices[i + 1].y = RADIUS * sinf(phi - rotation_offset + interval);
      vertices[i + 1].z = 0;
    }

    gl.getTransform() = glm::translate(gl.getTransform(), glm::vec3(width / 2, height / 2, 0));
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    setColor(255, 255, 255);
    gl.lines(vertices, (NUM_VERTS * 2));

    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;

    float apothem = cos(DEMOLOOP_M_PI / NUM_VERTS) * RADIUS * 2;
    float side = 2 * apothem * tan(DEMOLOOP_M_PI / NUM_VERTS);

    for (int i = 0; i < NUM_VERTS; ++i) {
      GL::TempTransform t1(gl);

      float internal_cycle_ratio = fmod(cycle_ratio * NUM_VERTS, 1);
      int current_vertex = fmod(floor(i + cycle_ratio * NUM_VERTS), NUM_VERTS);
      float x = cosf(current_vertex * interval - rotation_offset) * RADIUS;
      float y = sinf(current_vertex * interval - rotation_offset) * RADIUS;
      float phi = current_vertex * interval - rotation_offset + DEMOLOOP_M_PI * 2 / NUM_VERTS + DEMOLOOP_M_PI / 6;

      float x1 = x + side / 2 * cosf(phi) * internal_cycle_ratio;
      float y1 = y + side / 2 * sinf(phi) * internal_cycle_ratio;

      t1.get() = glm::translate(t1.get(), {x1, y1, 0});
      t1.get() = glm::rotate(t1.get(), static_cast<float>(DEMOLOOP_M_PI), {0, 0, 1});

      float rot = pow(internal_cycle_ratio, 4) * DEMOLOOP_M_PI * 2 / NUM_VERTS;
      t1.get() = glm::rotate(t1.get(), rot, {0, 0, 1});
      t1.get() = glm::translate(t1.get(), {-x, -y, 0});

      setColor(hsl2rgb(static_cast<float>(i) / NUM_VERTS, 1, 0.5));
      gl.lines(vertices, (NUM_VERTS * 2));
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
