#include <iostream>
#include <algorithm>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "demoloop.h"
#include "helpers.h"
#include "math_helpers.h"
#include "hsl.h"
using namespace std;

#define MAX_VERTS 6

float t = 0;
const float CYCLE_LENGTH = 10;

class Loop6 : public Demoloop {
public:
  Loop6() : Demoloop(150, 150, 150) {}

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    int ox = width / 2, oy = height / 2;

    const float vertex_cycle = sin(cycle_ratio * PI) * sin(cycle_ratio * PI);
    const int num_vertices = min((int)(vertex_cycle * (MAX_VERTS - 3) + 3 + 1), MAX_VERTS);

    const float interpolated_num_vertices = vertex_cycle * (MAX_VERTS - 3) + 3;
    const float interpolated_interval = (PI * 2) / interpolated_num_vertices;

    const float angularOffset = -PI / 2;

    int16_t xCoords[MAX_VERTS];
    int16_t yCoords[MAX_VERTS];
    for (int i = 0; i < num_vertices; ++i) {
      float t = i;

      xCoords[i] = cos(interpolated_interval * t + angularOffset) * RADIUS + ox;
      yCoords[i] = sin(interpolated_interval * t + angularOffset) * RADIUS + oy;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    filledPolygonColor(renderer, xCoords, yCoords, num_vertices, rgb2uint32(color));

    color = hsl2rgb(fmod(cycle_ratio + 0.5, 1), 1, 0.5);
    const int dot_count = 50;
    for (float i = 0; i < dot_count; ++i) {
      float interval_cycle_ratio = fmod(i / dot_count + cycle_ratio, 1);

      const int INTERNAL_RADIUS = cos(PI / interpolated_num_vertices) * RADIUS;

      float x1 = cos(interval_cycle_ratio * PI * 2 + angularOffset) * INTERNAL_RADIUS;
      float y1 = sin(interval_cycle_ratio * PI * 2 + angularOffset) * INTERNAL_RADIUS;
      x1 += sin(interval_cycle_ratio * PI * 2 * (interpolated_num_vertices - 1)) * INTERNAL_RADIUS * 0.5;
      y1 += cos(interval_cycle_ratio * PI * 2 * (interpolated_num_vertices - 1)) * INTERNAL_RADIUS * 0.5;

      filledCircleColor(renderer, x1 + ox, y1 + oy, 2, rgb2uint32(color));
      aacircleColor(renderer, x1 + ox, y1 + oy, 2, rgb2uint32(color));
    }
  }
private:
};

int main(int, char**){
  Loop6 loop;
  loop.Run();

  return 0;
}
