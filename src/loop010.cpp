
#include <vector>
#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 3;

class Loop10 : public Demoloop {
public:
  Loop10() : Demoloop(150, 150, 150) {
    gl.getProjection() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    mesh = cube(0, 0, 0, 1);
    vector<Vertex> vertices;
    vertices.reserve(36);
    for (auto i : mesh->mIndices) {
      vertices.push_back(mesh->mVertices[i]);
    }
    mesh->mVertices = vertices;
    iota(mesh->mIndices.begin(), mesh->mIndices.end(), 0);
    mesh->buffer();
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    static const uint16_t NUM_VERTS = 36;

    for (int i = 0; i < NUM_VERTS; ++i) {
      const float t = i;
      const float interval_cycle_ratio = fmod(t / NUM_VERTS + cycle_ratio, 1);
      auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);

      Vertex &v = mesh->mVertices[i];
      v.r = color.r;
      v.g = color.g;
      v.b = color.b;
      v.a = 255;
    }

    gl.pushTransform();
    const float cameraX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 4;
    const float cameraY = pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2), 2) * 3;
    const float cameraZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    gl.getTransform() = glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    mesh->buffer();
    mesh->draw();

    gl.popTransform();
  }

private:
  Mesh *mesh;
};

int main(int, char**){
  Loop10 test;
  test.Run();

  return 0;
}
