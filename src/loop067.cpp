#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include "math_helpers.h"
#include "graphics/image.h"
#include <array>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;
const float radius = 5;

template<size_t N>
array<Vertex, N * 2> getWireframeGeometry(const array<Vertex, N> &in) {
  array<Vertex, N * 2> out;

  const uint32_t numTris = N / 3;
  for (uint32_t i = 0; i < numTris; ++i) {
    for (uint32_t j = 0; j < 3; ++j) {
      const uint32_t index = 6 * i + j * 2;

      const uint32_t index1 = i * 3 + j;
      out[index + 0].x = in[index1].x;
      out[index + 0].y = in[index1].y;
      out[index + 0].z = in[index1].z;

      const uint32_t index2 = i * 3 + (j + 1) % 3;
      out[index + 1].x = in[index2].x;
      out[index + 1].y = in[index2].y;
      out[index + 1].z = in[index2].z;
    }
  }

  return out;
}

const uint32_t num_stars = 400;

class Loop066 : public Demoloop {
public:
  Loop066() : Demoloop(CYCLE_LENGTH, 720, 720, 0, 0, 0) {
    gl.getProjection() = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    array<glm::vec3, num_stars> flat_stars;
    for (uint32_t i = 0; i < num_stars; ++i) {
      // glm::vec2 temp = glm::circularRand(1.0f);
      glm::vec2 temp = glm::gaussRand(glm::vec2(0), glm::vec2(1));
      flat_stars[i] = glm::vec3(temp.x, 0, temp.y);
    }

    array<glm::vec3, num_stars> round_stars;
    for (uint32_t i = 0; i < num_stars; ++i) {
      round_stars[i] = glm::ballRand(1.0f);
    }

    for (uint32_t i = 0; i < num_stars; ++i) {
      const glm::vec3 &current_star = flat_stars[i];
      glm::vec3 closest_star;
      float distance = FLT_MAX;

      for (uint32_t i = 0; i < num_stars; ++i) {
        const glm::vec3 &s = flat_stars[i];
        if (s != current_star) {
          const float cur_dist = glm::distance(current_star, s);
          if (cur_dist < distance) {
            closest_star = s;
            distance = cur_dist;
          }
        }
      }

      memcpy(&flat_vertices[i * 2], &current_star, sizeof(glm::vec3));
      memcpy(&flat_vertices[i * 2 + 1], &closest_star, sizeof(glm::vec3));
    }

    for (uint32_t i = 0; i < num_stars; ++i) {
      const glm::vec3 &current_star = round_stars[i];
      glm::vec3 closest_star;
      float distance = FLT_MAX;

      for (uint32_t i = 0; i < num_stars; ++i) {
        const glm::vec3 &s = round_stars[i];
        if (s != current_star) {
          const float cur_dist = glm::distance(current_star, s);
          if (cur_dist < distance) {
            closest_star = s;
            distance = cur_dist;
          }
        }
      }

      memcpy(&round_vertices[i * 2], &current_star, sizeof(glm::vec3));
      memcpy(&round_vertices[i * 2 + 1], &closest_star, sizeof(glm::vec3));
    }
  }

  ~Loop066() {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();


    const glm::vec3 eye = glm::vec3(2, 1, 14);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    glm::mat4 transform;
    transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0, 1, 0});
    // transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * -0.5f, {0, 1, 0});
    transform = glm::scale(transform, {radius, radius, radius});

    const float a = 0.15;
    const float mod_ratio = 1 - (2 * (abs(cycle_ratio - 0.5)));
    const float gain1 = ((1.f/a)-2.f)*(1.f-(2.f*mod_ratio));
    const float v  = (mod_ratio < .5) ? mod_ratio/(1+gain1) : (gain1-mod_ratio)/(gain1-1);

    for (uint32_t i = 0; i < num_stars * 2; ++i) {
      mixed_vertices[i] = mix(flat_vertices[i], round_vertices[i], v);
    }

    gl.lines(mixed_vertices.data(), mixed_vertices.size(), transform);
  }

private:
  array<Vertex, num_stars * 2> flat_vertices;
  array<Vertex, num_stars * 2> round_vertices;
  array<Vertex, num_stars * 2> mixed_vertices;
};

int main(int, char**){
  Loop066 test;
  test.Run();

  return 0;
}
