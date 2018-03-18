#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include "math_helpers.h"
#include "graphics/image.h"
#include <array>
#include "poly2tri.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;
const float radius = 5;

const uint32_t num_stars = 400;

class Loop066 : public Demoloop {
public:
  Loop066() : Demoloop(CYCLE_LENGTH, 720, 720, 0, 0, 0) {
    gl.getProjection() = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    array<glm::vec2, num_stars> flat_stars;
    for (uint32_t i = 0; i < num_stars; ++i) {
      // glm::vec2 temp = glm::circularRand(1.0f);
      // glm::vec2 temp = glm::gaussRand(glm::vec2(0), glm::vec2(1));
      // glm::vec2 temp = glm::diskRand(1.0f);
      // flat_stars[i] = glm::vec3(temp.x, temp.y, 0);

      // flat_stars[i] = glm::ballRand(1.0f);
      flat_stars[i] = glm::gaussRand(glm::vec2(0), glm::vec2(0.8));
    }

    // Vertex v;
    // for (uint32_t i = 0; i < num_stars; ++i) {
    //   const glm::vec3 &star = flat_stars[i];
    //   for (const glm::vec3 &other : flat_stars) {
    //     if (other != star && glm::distance(star, other) < 0.2) {
    //       memcpy(&v, &star, sizeof(glm::vec3));
    //       vertices.push_back(v);
    //       memcpy(&v, &other, sizeof(glm::vec3));
    //       vertices.push_back(v);
    //     }
    //   }
    // }

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  }

  ~Loop066() {
    glDeleteBuffers(1, &vbo);
  }

  void Update() {
    const glm::vec3 eye = glm::vec3(2, 1, 14);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    gl.useVertexAttribArrays(ATTRIBFLAG_POS);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glm::mat4 transform;
    // transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0, 0, 1});
    // transform = glm::rotate(transform, trail_length * static_cast<float>(i) / num_trail, {0, 0, 1});
    // transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * -0.5f, {0, 1, 0});
    transform = glm::scale(transform, {radius, radius, radius});

    // gl.points(vertices.data(), vertices.size(), transform);
    gl.prepareDraw(transform);
    // gl.drawArrays(GL_POINTS, 0, vertices.size());
    gl.drawArrays(GL_LINES, 0, vertices.size());
  }

private:
  vector<Vertex> vertices;
  GLuint vbo;
};

int main(int, char**){
  Loop066 test;
  test.Run();

  return 0;
}
