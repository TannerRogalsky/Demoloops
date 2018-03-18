#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3


const float CYCLE_LENGTH = 10;

class Loop027 : public Demoloop {
public:
  Loop027() : Demoloop(CYCLE_LENGTH, 150, 150, 150), RADIUS(10), maxD(width / 4) {
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

    const glm::vec3 twoDAxis = {0, 0 , 1};
    GL::TempTransform outsideTransform(gl);
    outsideTransform.get()  = glm::rotate(outsideTransform.get(), -cycle_ratio * (float)DEMOLOOP_M_PI * 2, twoDAxis);

    for (uint32_t i = 0; i < num; ++i) {
      const float t = i;
      const float armIndex = i % arms;
      const float i_cycle_ratio = fmod(t / num + cycle_ratio, 1);

      float x = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * width / 4;
      x += pow(sinf(i_cycle_ratio * DEMOLOOP_M_PI), 2) * width / 10;
      float y = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * height / 4 * i_cycle_ratio;
      y += pow(sinf(i_cycle_ratio * DEMOLOOP_M_PI), 2) * width / 10;
      float d = sqrt(x * x + y * y);

      GL::TempTransform transform(gl);
      glm::mat4 &m = transform.get();
      m = glm::rotate(m, (float)DEMOLOOP_M_PI * 2 / arms * armIndex, twoDAxis);
      m = glm::translate(m, {x, y, i_cycle_ratio + 1});
      // transform.get().rotate(i_cycle_ratio * DEMOLOOP_M_PI * 2);

      setColor(hsl2rgb(fmod(cycle_ratio + d / maxD * 0.65, 1), 1, 0.5));
      gl.triangles(vertices, NUM_VERTS);
    }
  }
private:
  const float RADIUS;
  Vertex vertices[NUM_VERTS * 2];
  float maxD;
};

int main(int, char**){
  Loop027 loop;
  loop.Run();

  return 0;
}
