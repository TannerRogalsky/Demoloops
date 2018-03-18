#include "demoloop.h"
#include <glm/gtx/rotate_vector.hpp>
#include <vector>
#include <array>
#include <numeric>
#include <algorithm>
using namespace std;
using namespace demoloop;

const uint32_t CYCLE_LENGTH = 0.5;

glm::vec2 reflect(const glm::vec2 &v, const float &phi) {
  const float slope = cosf(phi) / sinf(phi);
  const float d = (v.x + v.y*slope)/(1 + powf(slope, 2));
  return {
    2*d - v.x,
    2*d*slope - v.y
  };
}

float randFloat() {
  return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

class Loop030 : public Demoloop {
public:
  Loop030() : Demoloop(CYCLE_LENGTH, 150, 150, 150), radius(width / 3) {
    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update() {
    glm::vec3 twoDAxis = {0, 0, 1};

    srand(floor(getTime() / CYCLE_LENGTH));
    // srand(t * 1000);
    rand();
    const uint32_t NUM_VERTS = floor(randFloat() * (8 - 5) + 5);
    const float interval = DEMOLOOP_M_PI / NUM_VERTS;
    const uint32_t NUM_POINTS = 3 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6.0-3.0)));

    vector<glm::vec2> control_points(NUM_POINTS + 1);
    const float control_point_interval = interval / NUM_POINTS;
    for (uint32_t i = 0; i <= NUM_POINTS; ++i) {
      control_points[i] = {
        cosf(control_point_interval * i) * radius * randFloat(), sinf(control_point_interval * i) * radius * randFloat()
      };
    }

    vector<Triangle> triangles((control_points.size() - 1) * 2);
    for (uint32_t i = 0; i < control_points.size() - 1; ++i) {
      uint32_t other_i = (i + 1) % control_points.size();
      glm::vec2 a = control_points[i];
      glm::vec2 b = control_points[other_i];
      triangles[i * 2] = {
        {0, 0, 1},
        {a.x, a.y, 1},
        {b.x, b.y, 1}
      };

      glm::vec2 ra = reflect(a, interval);
      glm::vec2 rb = reflect(b, interval);
      triangles[i * 2 + 1] = {
        {0, 0, 1},
        {ra.x, ra.y, 1},
        {rb.x, rb.y, 1}
      };
    }

    float phi = 0;
    for (uint32_t i = 0; i < NUM_VERTS; ++i, phi += (interval * 2)) {
      // setColor(hsl2rgb((float)i / NUM_VERTS, 1, 0.5));
      gl.triangles(triangles.data(), triangles.size(), glm::rotate(glm::mat4(), phi + (float)DEMOLOOP_M_PI / NUM_VERTS, twoDAxis));
    }
  }
private:
  float radius;
};

int main(int, char**){
  Loop030 loop;
  loop.Run();

  return 0;
}
