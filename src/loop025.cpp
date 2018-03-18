#include <algorithm>
#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3


const float CYCLE_LENGTH = 10;

class Loop024 : public Demoloop {
public:
  Loop024() : Demoloop(CYCLE_LENGTH, 150, 150, 150), RADIUS(10) {
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;
    float phi = 0.0f;
    for (int i = 0; i < NUM_VERTS; i++, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 1;
    }

    gl.getTransform() = glm::translate(gl.getTransform(), {0, height / 2, 0});
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const uint32_t num = width / (RADIUS * 2);

    for (uint32_t i = 0; i < num; ++i) {
      const float t = i;
      const float i_cycle_ratio = fmod(t / num / 2 + cycle_ratio, 1);

      const float x = i_cycle_ratio * width;
      const float y = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2 * 5) * height / 4 * i_cycle_ratio;

      GL::TempTransform transform(gl);
      transform.get() = glm::translate(transform.get(), {x, y, i_cycle_ratio + 1});

      setColor(hsl2rgb(t / num, 1, 0.5));
      gl.triangles(vertices, NUM_VERTS);
    }
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
