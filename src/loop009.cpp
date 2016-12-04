
#include <algorithm>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define MAX_VERTS 6

float t = 0;
const float CYCLE_LENGTH = 10;

class Loop9 : public Demoloop {
public:
  Loop9() : Demoloop(150, 150, 150) {
    glDisable(GL_DEPTH_TEST);
  }

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    int ox = width / 2, oy = height / 2;

    const float vertex_cycle = sin(cycle_ratio * DEMOLOOP_M_PI) * sin(cycle_ratio * DEMOLOOP_M_PI);
    const int num_vertices = min((int)(vertex_cycle * (MAX_VERTS - 3) + 3 + 1), MAX_VERTS);

    const float interpolated_num_vertices = vertex_cycle * (MAX_VERTS - 3) + 3;
    const float interpolated_interval = (DEMOLOOP_M_PI * 2) / interpolated_num_vertices;

    const float angularOffset = -DEMOLOOP_M_PI / 2;

    float xCoords[MAX_VERTS];
    float yCoords[MAX_VERTS];
    for (int i = 0; i < num_vertices; ++i) {
      float t = i;

      xCoords[i] = cos(interpolated_interval * t + angularOffset) * RADIUS + ox;
      yCoords[i] = sin(interpolated_interval * t + angularOffset) * RADIUS + oy;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    setColor(color);
    polygon(gl, xCoords, yCoords, num_vertices);

    color = hsl2rgb(fmod(cycle_ratio + 0.5, 1), 1, 0.5);
    setColor(color);
    const int dot_count = 50;
    for (float i = 0; i < dot_count; ++i) {
      float interval_cycle_ratio = fmod(i / dot_count + cycle_ratio, 1);

      const int INTERNAL_RADIUS = cos(DEMOLOOP_M_PI / interpolated_num_vertices) * RADIUS;

      float x1 = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 2 + angularOffset) * INTERNAL_RADIUS;
      float y1 = sin(interval_cycle_ratio * DEMOLOOP_M_PI * 2 + angularOffset) * INTERNAL_RADIUS;
      x1 += sin(interval_cycle_ratio * DEMOLOOP_M_PI * 2 * (interpolated_num_vertices - 1)) * INTERNAL_RADIUS * 0.5;
      y1 += cos(interval_cycle_ratio * DEMOLOOP_M_PI * 2 * (interpolated_num_vertices - 1)) * INTERNAL_RADIUS * 0.5;

      circle(gl, x1 + ox, y1 + oy, 2);
    }
  }
private:
};

int main(int, char**){
  Loop9 loop;
  loop.Run();

  return 0;
}
