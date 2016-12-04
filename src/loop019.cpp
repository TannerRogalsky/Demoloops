
#include <cmath>
#include <algorithm>
#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 6;
static const float RADIUS = 0.3;

class Loop19 : public Demoloop {
public:
  Loop19() : Demoloop(150, 150, 150) {
    gl.getProjection() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    mesh = sphere(0, 0, 0, RADIUS);
    auto indices = mesh->getIndexedVertices();
    Vertex *indexedVertices[12];
    uint32_t index = 0;
    for (auto i : indices) {
      indexedVertices[index++] = &mesh->mVertices[i];
    }
    std::sort(&indexedVertices[0], &indexedVertices[11], [](Vertex* a, Vertex* b) {
        return b->z < a->z;
    });

    for (Vertex *v : indexedVertices) {
      auto color = hsl2rgb((v->z + RADIUS) / (RADIUS * 4), 1, 0.5);
      v->r = color.r;
      v->g = color.g;
      v->b = color.b;
    }

    polygonLines = mesh->getLines();
    for (Vertex &v : polygonLines) {
      v.r = 0;
      v.g = 0;
      v.b = 0;
    }
    mesh->buffer();
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    gl.pushTransform();
    const float lookX = sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float lookY = 0;//cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float lookZ = cos(cycle_ratio * DEMOLOOP_M_PI * 2) * 3;
    const float cameraX = 0;//pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2) * 3;
    const float cameraY = 0;
    const float cameraZ = 0;
    gl.getTransform() = glm::lookAt(glm::vec3(lookX, lookY, lookZ), glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(0, 1, 0));

    // setColor(255, 255, 255);
    mesh->draw();

    gl.lines(polygonLines.data(), polygonLines.size());

    const uint32_t numLines = 12 * 2 * 2;
    Vertex lines[numLines];
    uint32_t index = 0;
    for (int i = 0; i < 12; ++i) {
      const float t = i;
      const auto color = hsl2rgb(t / 12, 1, 0.5);

      lines[index].x = 0;
      lines[index].y = 0;
      lines[index].z = 0;
      lines[index].r = 0;
      lines[index].g = 0;
      lines[index].b = 0;
      index++;

      lines[index].x = 0;
      lines[index].y = sinf(i * DEMOLOOP_M_PI * 2 / 12);
      lines[index].z = cosf(i * DEMOLOOP_M_PI * 2 / 12);
      lines[index].r = color.r;
      lines[index].g = color.g;
      lines[index].b = color.b;
      index++;
    }

    for (int i = 0; i < 12; ++i) {
      lines[index].x = 0;
      lines[index].y = sinf(i * DEMOLOOP_M_PI * 2 / 12);
      lines[index].z = cosf(i * DEMOLOOP_M_PI * 2 / 12);
      index++;

      lines[index].x = 0;
      lines[index].y = sinf((i + 1) * DEMOLOOP_M_PI * 2 / 12);
      lines[index].z = cosf((i + 1) * DEMOLOOP_M_PI * 2 / 12);
      index++;
    }

    gl.getTransform() = glm::rotate(gl.getTransform(), cycle_ratio * (float)DEMOLOOP_M_PI * 2, {0, 0, 1});
    gl.lines(lines, numLines);

    gl.popTransform();
  }

private:
  Mesh *mesh;
  vector<Vertex> polygonLines;
};

int main(int, char**){
  Loop19 test;
  test.Run();

  return 0;
}
