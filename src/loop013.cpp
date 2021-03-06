#include <glm/gtx/rotate_vector.hpp>
#include <cmath>
#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

static const uint16_t NUM_VERTS = 60;
const uint32_t CYCLE_LENGTH = 8;
static const float RADIUS = 0.3;

class Loop13 : public Demoloop {
public:
  Loop13() : Demoloop(CYCLE_LENGTH, 150, 150, 150), mesh(icosahedron(0, 0, 0, RADIUS)) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    iota(mesh.mIndices.begin(), mesh.mIndices.end(), 0);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    for (int i = 0; i < NUM_VERTS; ++i) {
      const float t = i;
      const float interval_cycle_ratio = fmod(t / NUM_VERTS + cycle_ratio, 1);
      auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);

      Vertex &v = mesh.mVertices[i];
      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
      v.a = 255;
    }

    gl.pushTransform();
    const float cameraX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    // const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2);
    const float cameraY = 0;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    gl.getTransform() = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    mesh.buffer();
    mesh.draw();

    static const float SCALE = 0.2;
    gl.getTransform() = glm::scale(gl.getTransform(), {SCALE, SCALE, SCALE});
    const uint16_t numPoints = 50;
    for (int i = 0; i < numPoints; ++i) {
      gl.pushTransform();
      const float t = i;
      const float interval_cycle_ratio = fmod(t / numPoints + cycle_ratio, 1);

      const float x = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 2) * RADIUS * 4 / SCALE;
      const float y = sin(interval_cycle_ratio * DEMOLOOP_M_PI * 2) * RADIUS * 3 / SCALE;
      // const float y = sin(t / numPoints * DEMOLOOP_M_PI * 2 * interval_cycle_ratio) * RADIUS * 3 / SCALE;
      const float z = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 6) * RADIUS * 3 / SCALE;
      gl.getTransform() = glm::translate(gl.getTransform(), {x, y, z});
      mesh.draw();
      gl.popTransform();
    }

    gl.popTransform();
  }

private:
  // Vertex vertices[NUM_VERTS];
  Mesh mesh;
};

int main(int, char**){
  Loop13 test;
  test.Run();

  return 0;
}
