#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 3
#define RADIUS 10
const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;
const Triangle triangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 1},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 1},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 1}
};


const uint32_t CYCLE_LENGTH = 10;
const uint32_t arms = 5;
const uint32_t num = arms * 20;

class Loop030 : public Demoloop {
public:
  Loop030() : Demoloop(CYCLE_LENGTH, 150, 150, 150), maxD(width / 4) {
    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    glm::vec3 twoDAxis = {0, 0 , 1};
    for (uint32_t i = 0; i < num; ++i) {
      const float t = i;
      const float armIndex = i % arms;
      const float i_cycle_ratio = fmod(t / num + cycle_ratio, 1);

      float x = cosf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * width / 4;
      x *= sinf(cycle_ratio * DEMOLOOP_M_PI * 2);
      x += pow(sinf(i_cycle_ratio * DEMOLOOP_M_PI), 2) * width / 10;
      float y = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * width / 4;
      y *= cosf(cycle_ratio * DEMOLOOP_M_PI * 2);
      float d = sqrt(x * x + y * y);

      glm::mat4 m;
      m = glm::rotate(m, -(float)DEMOLOOP_M_PI / 2, twoDAxis);
      m = glm::rotate(m, (float)DEMOLOOP_M_PI * 2 / arms * armIndex, twoDAxis);
      m = glm::translate(m, {x, y, 1});

      triangles[i] = triangle;
      applyMatrix(triangles[i], m);
      applyColor(triangles[i], hsl2rgb(fmod(cycle_ratio + d / maxD * 0.65, 1), 1, 0.5));
    }
    gl.triangles(triangles, num);
  }
private:
  Triangle triangles[num];
  float maxD;
};

int main(int, char**){
  Loop030 loop;
  loop.Run();

  return 0;
}
