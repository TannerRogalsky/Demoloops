
#include <cmath>
#include <algorithm>
#include <numeric>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

class Loop20 : public Demoloop {
public:
  Loop20() : Demoloop(CYCLE_LENGTH, 150, 150, 150), mesh(icosahedron(0, 0, 0, 1.2)) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI / 4.0f), (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    points = mesh.getIndexedVertices();
    float t = 0;
    for (auto i : points) {
      auto color = hsl2rgb(t++ / points.size(), 1, 0.5);
      Vertex &v = mesh.mVertices[i];
      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
    }
    mesh.buffer();

    lines = mesh.getLines();
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const float rad = cycle_ratio * DEMOLOOP_M_PI * 2;

    const glm::vec3 rotationAxis = glm::normalize(glm::vec3(-1, sinf(rad) / 3, 0));
    const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 3 + pow(sinf(pow(cycle_ratio, 3) * DEMOLOOP_M_PI), 2) * 10), rad, rotationAxis);
    const glm::vec3 up = glm::rotate(glm::vec3(0, 1, 0), rad, rotationAxis);
    const glm::vec3 target = glm::rotate(glm::vec3(0, 2, 0), rad, rotationAxis);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    setColor(255, 255, 255);
    mesh.draw();
    setColor(0, 0, 0);
    gl.lines(lines.data(), lines.size());

    for (auto i : points) {
      Vertex &v = mesh.mVertices[i];

      GL::TempTransform t2(gl);
      t2.get() = glm::translate(t2.get(), glm::vec3(v.x * 1.1, v.y * 1.1, v.z * 1.1));
      t2.get() = glm::scale(t2.get(), glm::vec3(0.1, 0.1, 0.1));
      t2.get() = glm::rotate(t2.get(), rad * 2, glm::vec3(0, 1, 0));

      setColor(255, 255, 255);
      mesh.draw();
      setColor(0, 0, 0);
      gl.lines(lines.data(), lines.size());
    }
  }

private:
  Mesh mesh;
  set<uint32_t> points;
  vector<Vertex> lines;
};

int main(int, char**){
  Loop20 test;
  test.Run();

  return 0;
}
