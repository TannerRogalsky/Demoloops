
#include <algorithm>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3


const uint32_t CYCLE_LENGTH = 10;

class Loop026 : public Demoloop {
public:
  Loop026() : Demoloop(CYCLE_LENGTH, 150, 150, 150), RADIUS(10), maxD(width / 4) {
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;
    float phi = 0.0f;
    for (int i = 0; i < NUM_VERTS; i++, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 1;
    }

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const uint32_t arms = 7;
    const uint32_t num = arms * 10;

    for (uint32_t i = 0; i < num; ++i) {
      const float t = i;
      const float armIndex = i % arms;
      const float i_cycle_ratio = fmod(t / num + cycle_ratio, 1);

      const float x = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2 * 2) * width / 4;
      const float y = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * height / 4 * i_cycle_ratio;
      const float d = sqrt(x * x + y * y);

      GL::TempTransform transform(gl);
      transform.get() = glm::rotate(transform.get(), (float)DEMOLOOP_M_PI * 2 / arms * armIndex, {0, 0, 1});
      transform.get() = glm::translate(transform.get(), {x, y, i_cycle_ratio + 1});

      setColor(hsl2rgb(d / maxD * 0.65, 1, 0.5));
      gl.triangles(vertices, NUM_VERTS);
    }
  }
private:
  const float RADIUS;
  Vertex vertices[NUM_VERTS * 2];
  float maxD;
};

int main(int, char**){
  Loop026 loop;
  loop.Run();

  return 0;
}
