#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

const uint32_t NUM_VERTS = 3;

float t = 0;
const float CYCLE_LENGTH = 6;

template <
  typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
> constexpr T mix(T const &a, T const &b, const float &ratio) {
  return a * (1.0f - ratio) + b * ratio;
}

class Loop021 : public Demoloop {
public:
  Loop021() : Demoloop(150, 150, 150), RADIUS(height / 6) {
    glDisable(GL_DEPTH_TEST);

    float phi = 0.0f;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;

    for (uint32_t i = 0; i < (NUM_VERTS * 2) - 1; i+=2, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 0;

      vertices[i + 1].x = RADIUS * cosf(phi + interval);
      vertices[i + 1].y = RADIUS * sinf(phi + interval);
      vertices[i + 1].z = 0;
    }

    gl.getTransform() = glm::translate(gl.getTransform(), glm::vec3(width / 2, height / 2, 0));
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    setColor(255, 255, 255);
    gl.lines(vertices, (NUM_VERTS * 2));

    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;

    const glm::vec3 twoDAxis = {0, 0, 1};

    for (uint32_t i = 0; i < NUM_VERTS; ++i) {
      float internal_cycle_ratio = fmod(cycle_ratio * NUM_VERTS, 1);
      int current_vertex = fmod(floor(i + cycle_ratio * NUM_VERTS), NUM_VERTS);
      float x1 = cosf(current_vertex * interval) * RADIUS;
      float y1 = sinf(current_vertex * interval) * RADIUS;

      float x2 = cosf((current_vertex + 1) * interval) * RADIUS;
      float y2 = sinf((current_vertex + 1) * interval) * RADIUS;

      float x = mix(x1, x2, internal_cycle_ratio);
      float y = mix(y1, y2, internal_cycle_ratio);

      glm::mat4 m;
      m = glm::translate(m, {x, y, 0});
      m = glm::rotate(m, (float)DEMOLOOP_M_PI * cycle_ratio * 2, twoDAxis);
      m = glm::rotate(m, i * (float)DEMOLOOP_M_PI * 2 / NUM_VERTS, twoDAxis);
      m = glm::translate(m, {RADIUS, 0, 0});
      m = glm::rotate(m, (float)DEMOLOOP_M_PI, twoDAxis);

      setColor(hsl2rgb(static_cast<float>(i) / NUM_VERTS, 1, 0.5));
      gl.lines(vertices, (NUM_VERTS * 2), m);
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
