
#include <cmath>
#include <algorithm>
#include <numeric>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;


const float CYCLE_LENGTH = 10;
const uint32_t numTris = 300;

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
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI / 4.0f), (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const float rad = cycle_ratio * DEMOLOOP_M_PI * 2;

    const glm::vec3 eye = glm::vec3(0, 0, 2);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    srand(0);

    for (uint32_t i = 0; i < numTris; ++i) {
      const float t = i;
      // glm::vec3 p = glm::sphericalRand(1.0f);
      glm::vec3 p = glm::gaussRand(glm::vec3(0), glm::vec3(1));
      p = glm::rotate(p, -rad, glm::vec3(0, 1, 0));
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
