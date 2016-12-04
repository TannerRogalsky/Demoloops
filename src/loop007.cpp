
#include <cmath>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define MAX_VERTS 6

float t = 0;
const float CYCLE_LENGTH = 6;

class Loop7 : public Demoloop {
public:
  Loop7() : Demoloop(150, 150, 150) {
    glDisable(GL_DEPTH_TEST);

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 6;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    const uint8_t num_vertices = MAX_VERTS;

    const float interval = (DEMOLOOP_M_PI * 2) / num_vertices;
    float xCoords[MAX_VERTS];
    float yCoords[MAX_VERTS];
    for (int i = 0; i < num_vertices; ++i) {
      float t = i;
      xCoords[i] = cos(interval * t) * RADIUS;
      yCoords[i] = sin(interval * t) * RADIUS;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    setColor(color);
    polygon(gl, xCoords, yCoords, num_vertices);

    color = hsl2rgb(fmod(cycle_ratio + 0.5, 1), 1, 0.5);
    setColor(color, 255 / 6);

    for (int i = 0; i < 6; i+=1) {
      const float t = i;

      const float phi = cycle_ratio * DEMOLOOP_M_PI * 2;
      const float c = cos(phi);
      const float s = sin(phi);

      const float cx = cos(interval * t) * RADIUS;
      const float cy = sin(interval * t) * RADIUS;

      for (int j = 0; j < num_vertices; ++j) {
        const float u = j;
        const float px = cos(interval * u) * RADIUS;
        const float py = sin(interval * u) * RADIUS;

        const float dx = px - cx;
        const float dy = py - cy;

        xCoords[j] = cx + (dx * c - dy * s);
        yCoords[j] = cy + (dx * s + dy * c);
      }
      polygon(gl, xCoords, yCoords, num_vertices);
    }
  }

private:
};

int main(int, char**){
  Loop7 loop;
  loop.Run();

  return 0;
}
