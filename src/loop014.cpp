#include <glm/gtx/rotate_vector.hpp>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

static const uint16_t NUM_VERTS = 60;
const uint32_t CYCLE_LENGTH = 6;
static const float RADIUS = 0.3;

class Loop14 : public Demoloop {
public:
  Loop14() : Demoloop(CYCLE_LENGTH, 150, 150, 150), mesh(icosahedron(0, 0, 0, RADIUS)) {
    gl.getProjection() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    points = mesh.getIndexedVertices();
    iota(mesh.mIndices.begin(), mesh.mIndices.end(), 0);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    // sort(&vertices[0], &vertices[NUM_VERTS], [](Vertex a, Vertex b) {
    //     return b.z < a.z;
    // });


    const float color_cycle = pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2);
    for (int i = 0; i < NUM_VERTS; ++i) {
      Vertex &v = mesh.mVertices[i];
      // const float t = i;
      // const float interval_cycle_ratio = fmod(t / NUM_VERTS + cycle_ratio, 1);
      auto color = hsl2rgb(((v.z + RADIUS + v.x + RADIUS + v.y + RADIUS) / (RADIUS * 6) + color_cycle) / 2, 1, 0.5);

      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
      v.a = 255;
    }
    mesh.buffer();

    gl.pushTransform();
    const float cameraX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    // const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2);
    const float cameraY = 0;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    gl.getTransform() = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    setColor(255, 255, 255);
    mesh.draw();

    setColor(0, 0, 0);

    Vertex lines[12 * 11 * 2];
    uint32_t index = 0;
    for (int i = 0; i < 12; ++i) {
      for (int j = 0; j < 12; ++j) {
        if (i != j) {
          lines[index++] = mesh.mVertices[*std::next(points.begin(), i)];
          lines[index++] = mesh.mVertices[*std::next(points.begin(), j)];
        }
      }
    }
    gl.lines(lines, 12 * 11 * 2);

    gl.popTransform();
  }

private:
  Mesh mesh;
  set<uint32_t> points;
};

int main(int, char**){
  Loop14 test;
  test.Run();

  return 0;
}
