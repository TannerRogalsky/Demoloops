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
const float CYCLE_LENGTH = 3;

template<uint32_t N>
typename std::enable_if<N >= 3, array<Vertex, N>>::type
polygonVertices(const float &radius) {
  array<Vertex, N> r;

  const float interval = DEMOLOOP_M_PI * 2 / N;
  float phi = 0.0f;
  for (uint32_t i = 0; i < N; ++i, phi += interval) {
    r[i].x = cosf(phi) * radius;
    r[i].y = sinf(phi) * radius;
    r[i].z = 1;

    // RGB c = hsl2rgb(phi / (DEMOLOOP_M_PI * 2), 1, 0.5);
    // r[i].r = c.r;
    // r[i].g = c.g;
    // r[i].b = c.b;
  }

  return r;
}

float getComplimentaryScale(float radius, uint32_t primary_num_verts, uint32_t secondary_num_verts, float primary_scale) {
  const float previous_side_length = sinf(DEMOLOOP_M_PI / primary_num_verts) * 2 * radius * primary_scale;
  const float current_side_length = sinf(DEMOLOOP_M_PI / secondary_num_verts) * 2 * radius;
  return previous_side_length / current_side_length;
}

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(CYCLE_LENGTH, 1280, 720, 9,103,131) {
    // glDisable(GL_DEPTH_TEST);
    // glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    // gl.getProjection() = perspective;

    // gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
    gl.getTransform() = glm::translate(gl.getTransform(), {0, -height / 2, 0});

    // m = glm::translate(m, {0, 0, 30});
  }

  ~Loop055() {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();
    // const uint32_t num_cycles = 2;
    // const float cycle_ratio = 0.5;

    const uint32_t NUM_VERTS = 12;
    const float RADIUS = height * 0.1;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS;


    const float ratio = getComplimentaryScale(RADIUS, NUM_VERTS, 3, 1);

    const float x_offset = RADIUS * cosf(DEMOLOOP_M_PI / NUM_VERTS);
    const float y_offset = RADIUS * cosf(DEMOLOOP_M_PI / NUM_VERTS) * cosf(interval);
    for (int32_t y_index = -2; y_index < 6; ++y_index) {
      for (int32_t x_index = -2; x_index < 11; ++x_index) {
        const float direction = ((y_index % 2) - 0.5) * 2.0;
        // const float x = x_offset * 2 * x_index + (y_index % 2) * x_offset - x_offset * 2 * cycle_ratio * direction;
        const float x = x_offset * 2 * x_index;
        const float y = y_offset * 2 * y_index * 2;

        glm::mat4 view;
        view = glm::translate(view, {x, y, 0});
        const float d = RADIUS * ratio * (1.0 - powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2.0));
        view = glm::translate(view, {
          cosf(cycle_ratio * DEMOLOOP_M_PI * 2 + x_index + y_index * 11) * d,
          sinf(cycle_ratio * DEMOLOOP_M_PI * 2 + x_index + y_index * 11) * d,
        0});
        view = glm::rotate(view, rotationOffset(NUM_VERTS), {0, 0, 1});
        view = glm::scale(view, {
          0.5 + (0.25 * powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2.0)),
          0.5 + (0.25 * powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2.0)),
        1});
        setColor(225,97,130);
        gl.triangleFan(polygonVertices<NUM_VERTS>(RADIUS).data(), NUM_VERTS, view);

        {
          setColor(246,190,55);
          view = glm::translate(glm::mat4(), {x + x_offset, y + y_offset, 1});
          view = glm::rotate(view, rotationOffset(3) + (float)DEMOLOOP_M_PI, {0, 0, 1});
          view = glm::scale(view, {ratio, ratio, 1});
          gl.triangleFan(polygonVertices<3>(RADIUS).data(), 3, view);

          view = glm::translate(glm::mat4(), {x + x_offset, y - y_offset, 1});
          view = glm::rotate(view, rotationOffset(3), {0, 0, 1});
          view = glm::scale(view, {ratio, ratio, 1});
          gl.triangleFan(polygonVertices<3>(RADIUS).data(), 3, view);
        }


        {
          const float hex_ratio = getComplimentaryScale(RADIUS, 3, 6, ratio);
          const float previous_distance = RADIUS * cosf(DEMOLOOP_M_PI / 3) * ratio;
          const float current_distance = RADIUS * cosf(DEMOLOOP_M_PI / 6) * hex_ratio;
          const float d = previous_distance + current_distance;

          const float y_travel = sinf(cycle_ratio * DEMOLOOP_M_PI) * current_distance;
          const float x_travel = cycle_ratio * x_offset * 2 * direction;
          view = glm::translate(glm::mat4(), {x + x_offset - x_travel, y - y_offset - d - y_travel, 0});
          view = glm::rotate(view, (float)DEMOLOOP_M_PI * cycle_ratio * -direction, {0, 0, 1});
          view = glm::scale(view, {hex_ratio, hex_ratio, 1});

          setColor(50,208,188);
          gl.triangleFan(polygonVertices<6>(RADIUS).data(), 6, view);
        }
      }
    }
  }

private:
  glm::mat4 m;
};

int main(int, char**){
  Loop055 test;
  test.Run();

  return 0;
}
