#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include <glm/gtx/rotate_vector.hpp>
#include "hsl.h"
using namespace std;
using namespace demoloop;

static const uint16_t NUM_VERTS = 60;

const float CYCLE_LENGTH = 3;
static const float RADIUS = 0.3;

class Loop11 : public Demoloop {
public:
  Loop11() : Demoloop(CYCLE_LENGTH, 150, 150, 150), mesh(icosahedron(0, 0, 0, RADIUS)) {
    gl.getProjection() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

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
    const float cameraX = 0;//sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    // const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2);
    const float cameraY = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraZ = 3;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    gl.getTransform() = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    mesh.buffer();
    mesh.draw();
    glm::mat4 m;
    m = glm::translate(m, {0, sinf(cycle_ratio * DEMOLOOP_M_PI * 2) * RADIUS * 4, 0});
    m = glm::scale(m, {0.5, 0.5, 0.5});
    mesh.draw(m);

    gl.popTransform();
  }

private:
  Mesh mesh;
};

int main(int, char**){
  Loop11 test;
  test.Run();

  return 0;
}
