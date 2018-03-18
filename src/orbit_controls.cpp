#include <vector>
#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include <glm/gtx/rotate_vector.hpp>
#include "hsl.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 3;

class Loop10 : public Demoloop {
public:
  Loop10() : Demoloop(CYCLE_LENGTH, 150, 150, 150), mesh(cube(0, 0, 0, 10)) {
    gl.getProjection() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    for (auto i : mesh.mIndices) {
      auto color = hsl2rgb(static_cast<float>(i) / mesh.mIndices.size(), 1, 0.5);
      applyColor(mesh.mVertices[i], color);
    }
    mesh.buffer();
  }

  void Update() {
    glm::mat4 &m = gl.getTransform();

    if (isMouseDown(SDL_BUTTON_RIGHT)) {
      m = glm::translate(m, glm::vec3(getMouseDeltaX(), getMouseDeltaY(), 0));
    }
    if (isMouseDown(SDL_BUTTON_LEFT)) {
      float rotateSpeed = 1;
      m = glm::rotate(m, 2 * static_cast<float>(DEMOLOOP_M_PI) * getMouseDeltaX() / width * rotateSpeed, glm::vec3(0, 1, 0));
      m = glm::rotate(m, 2 * static_cast<float>(DEMOLOOP_M_PI) * getMouseDeltaY() / height * rotateSpeed, glm::vec3(1, 0, 0));
    }

    // glm::mat4 m;
    // m = glm::translate(m, {0, 0, 10});
    // m = glm::rotate(m, (float)DEMOLOOP_M_PI * 2 * getCycleRatio(), {0, 1, 1});
    mesh.draw();
  }

private:
  Mesh mesh;
};

int main(int, char**){
  Loop10 test;
  test.Run();

  return 0;
}
