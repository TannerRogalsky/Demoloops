#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/2d_primitives.h"
#include <array>
using namespace std;
using namespace demoloop;

const uint32_t CYCLE_LENGTH = 10;

glm::mat4 bezier_mat = {{-1.f, 3.f, -3.f, 1.f}, {3.f, -6.f, 3.f, 0.f}, {-3.f, 3.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f}};
glm::vec2 evaluate(const array<glm::vec2, 4> &controlPoints, float t) {
  glm::vec4 T = {powf(t, 3), powf(t, 2), t, 1};
  glm::vec4 a = T * bezier_mat;

  return {
    glm::dot(a, glm::vec4(controlPoints[0].x, controlPoints[1].x, controlPoints[2].x, controlPoints[3].x)),
    glm::dot(a, glm::vec4(controlPoints[0].y, controlPoints[1].y, controlPoints[2].y, controlPoints[3].y))
  };
}

class Bezier : public Demoloop {
public:
  Bezier() : Demoloop(CYCLE_LENGTH, 150, 150, 150) {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    array<glm::vec2, 4> controlPoints = {{
      {100 * 5 - 400, 100 * 5 - 400},
      {100 * 5 - 400, 200 * 5 - 400},
      {200 * 5 - 400, 100 * 5 - 400},
      {100 * 5 - 400, 100 * 5 - 400},
    }};

    float length = 0;
    const uint32_t num_segments = 100;

    setColor(255, 255, 255);
    const float interval = 1.0/num_segments;
    for (uint32_t i = 0; i < num_segments; ++i) {
      float t = i * interval;
      float nt = (i + 1) * interval;
      glm::vec2 a = evaluate(controlPoints, t);
      glm::vec2 b = evaluate(controlPoints, nt);

      length += glm::distance(a, b);

      line(gl, a.x, a.y, b.x, b.y);
    }

    setColor(0, 255, 0);
    glm::vec2 p = evaluate(controlPoints, cycle_ratio);
    circle(gl, p.x, p.y, 3);
  }

private:
};

int main(int, char**){
  Bezier test;
  test.Run();

  return 0;
}
