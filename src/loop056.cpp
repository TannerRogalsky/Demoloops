#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/shader.h"
#include "helpers.h"
#include "math_helpers.h"
#include <array>
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(150, 150, 150) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    // gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  ~Loop055() {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;


    const uint32_t NUM_VERTS = 7;
    Vertex vertices[NUM_VERTS * 6];
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;
    const float RADIUS = 20;
    const float THICKNESS = 0.8;

    for (uint32_t i = 0; i < NUM_VERTS; ++i) {
      const uint32_t a = i;
      const uint32_t b = i + 2;

      const float phi1 = interval * a;
      const float phi2 = interval * b;

      vertices[i * 6 + 0].x = cosf(phi1) * RADIUS;
      vertices[i * 6 + 0].y = sinf(phi1) * RADIUS;
      vertices[i * 6 + 0].z = NUM_VERTS / 2;
      applyColor(vertices[i * 6 + 0], hsl2rgb(fmod(phi1 / DEMOLOOP_M_PI + cycle_ratio, 1), 1.0, 0.5));

      vertices[i * 6 + 1].x = cosf(phi1) * RADIUS * THICKNESS;
      vertices[i * 6 + 1].y = sinf(phi1) * RADIUS * THICKNESS;
      vertices[i * 6 + 1].z = NUM_VERTS / 2;
      applyColor(vertices[i * 6 + 1], hsl2rgb(fmod(phi1 / DEMOLOOP_M_PI + cycle_ratio, 1), 1.0, 0.5));

      vertices[i * 6 + 2].x = cosf(phi2) * RADIUS;
      vertices[i * 6 + 2].y = sinf(phi2) * RADIUS;
      vertices[i * 6 + 2].z = NUM_VERTS / -2;
      applyColor(vertices[i * 6 + 2], hsl2rgb(fmod(phi2 / DEMOLOOP_M_PI + cycle_ratio, 1), 1.0, 0.5));

      vertices[i * 6 + 3].x = cosf(phi1) * RADIUS * THICKNESS;
      vertices[i * 6 + 3].y = sinf(phi1) * RADIUS * THICKNESS;
      vertices[i * 6 + 3].z = NUM_VERTS / 2;
      applyColor(vertices[i * 6 + 3], hsl2rgb(fmod(phi1 / DEMOLOOP_M_PI + cycle_ratio, 1), 1.0, 0.5));

      vertices[i * 6 + 4].x = cosf(phi2) * RADIUS * THICKNESS;
      vertices[i * 6 + 4].y = sinf(phi2) * RADIUS * THICKNESS;
      vertices[i * 6 + 4].z = NUM_VERTS / -2;
      applyColor(vertices[i * 6 + 4], hsl2rgb(fmod(phi2 / DEMOLOOP_M_PI + cycle_ratio, 1), 1.0, 0.5));

      vertices[i * 6 + 5].x = cosf(phi2) * RADIUS;
      vertices[i * 6 + 5].y = sinf(phi2) * RADIUS;
      vertices[i * 6 + 5].z = NUM_VERTS / -2;
      applyColor(vertices[i * 6 + 5], hsl2rgb(fmod(phi2 / DEMOLOOP_M_PI + cycle_ratio, 1), 1.0, 0.5));
    }

    glm::mat4 view = glm::mat4();
    view = glm::translate(view, {0, 0, RADIUS});
    // view = glm::rotate(view, sinf(DEMOLOOP_M_PI * 2 * cycle_ratio), {0, 1, 0});
    view = glm::rotate(view, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0, 1, 0});
    // view = glm::rotate(view, rotationOffset(NUM_VERTS) + (float)DEMOLOOP_M_PI, {0, 0, 1});
    view = glm::rotate(view, rotationOffset(NUM_VERTS), {0, 0, 1});
    gl.triangles(vertices, NUM_VERTS * 6, view);
  }

private:
};

int main(int, char**){
  Loop055 test;
  test.Run();

  return 0;
}
