#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;


const uint32_t CYCLE_LENGTH = 10;
const uint32_t arms = 5;
const uint32_t numTris = arms * 20;

const float RADIUS = 0.05;
const float interval = DEMOLOOP_M_PI * 2 / 3;
const Triangle triangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 1},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 1},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 1}
};

class Loop034 : public Demoloop {
public:
  Loop034() : Demoloop(CYCLE_LENGTH, 150, 150, 150) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const glm::vec3 eye = glm::vec3(0, 0, 2);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    srand(0);

    for (uint32_t i = 0; i < numTris; ++i) {
      const float t = i;
      const float armIndex = i % arms;
      const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);

      glm::vec3 p(0, 0, 1);
      p = glm::rotate(p, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 0, 1));
      p = glm::rotate(p, (float)DEMOLOOP_M_PI * 2 / armIndex / arms, glm::vec3(0, 1, 0));
      p = glm::rotate(p, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(1, 0, 0));
      p = glm::rotate(p, (float)DEMOLOOP_M_PI  / 2, glm::vec3(0, 1, 0));
      p = glm::rotate(p, sinf(cycle_ratio * DEMOLOOP_M_PI * 2) * (float)DEMOLOOP_M_PI / 3, glm::vec3(0, 1, 0));
      glm::mat4 m = glm::lookAt(p, {0, 0, 0}, up);

      triangles[i] = triangle;
      applyMatrix(triangles[i], m);
      applyColor(triangles[i], hsl2rgb(t / numTris, 1, 0.5));
    }

    gl.triangles(triangles, numTris);
  }

private:
  Triangle triangles[numTris];
};

int main(int, char**){
  Loop034 test;
  test.Run();

  return 0;
}
