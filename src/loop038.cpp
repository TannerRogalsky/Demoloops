#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;


const uint32_t CYCLE_LENGTH = 10;
const uint32_t arms = 20;
const uint32_t trisPerArm = 20;
const uint32_t numTris = arms * trisPerArm;
// const uint32_t numTris = 20;

const float RADIUS = 0.05;
const float interval = DEMOLOOP_M_PI * 2 / 3;
const Triangle triangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 1},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 1},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 1}
};

class Loop038 : public Demoloop {
public:
  Loop038() : Demoloop(CYCLE_LENGTH, 150, 150, 150) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const glm::vec3 eye = glm::rotate(glm::vec3(4, 1, 4), -cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(-0.3, 1, 0));
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    srand(0);

    for (uint32_t i = 0; i < numTris; ++i) {
      const float t = i;
      const float armIndex = i % arms;
      const float i_cycle_ratio = fmod(floor(t / arms) / trisPerArm + cycle_ratio, 1);
      // const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);

      glm::vec4 p(0, 0, 0, 0);
      glm::mat4 m1;
      m1 = glm::rotate(m1, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 0, 1));
      m1 = glm::rotate(m1, armIndex / arms * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 0, 1));
      m1 = glm::rotate(m1, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
      m1 = glm::translate(m1, glm::vec3(0, 1, powf(sinf(cycle_ratio * DEMOLOOP_M_PI ), 2) * 3));

      // p = m1 * p;
      // glm::vec4 up4 = m1 * glm::vec4(up, 0);
      // glm::mat4 m2 = glm::lookAt(glm::vec3(p.x, p.y, p.z), {0, 0, 0}, glm::vec3(up4.x, up4.y, up4.z));

      triangles[i] = triangle;
      applyMatrix(triangles[i], m1);
      applyColor(triangles[i], hsl2rgb(t / numTris, 1, 0.5));
    }

    gl.triangles(triangles, numTris);
  }

private:
  Triangle triangles[numTris];
};

int main(int, char**){
  Loop038 test;
  test.Run();

  return 0;
}
