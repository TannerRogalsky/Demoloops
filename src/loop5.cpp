#include <iostream>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "demoloop.h"
#include "helpers.h"
#include "hsl.h"
using namespace std;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 10;

class Loop5 : public Demoloop {
public:
  Loop5() : Demoloop(150, 150, 150) {}

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    int ox = width / 2, oy = height / 2;

    const int num_vertices = 5;
    const float interval = (PI * 2) / num_vertices;
    int16_t xCoords[num_vertices];
    int16_t yCoords[num_vertices];
    for (int i = 0; i < num_vertices; ++i) {
      float t = i;
      xCoords[i] = cos(interval * t - PI / 10) * RADIUS + ox;
      yCoords[i] = sin(interval * t - PI / 10) * RADIUS + oy;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    filledPolygonColor(renderer, xCoords, yCoords, num_vertices, rgb2uint32(color));
    const int dot_count = 20;
    for (int v = 0; v < num_vertices; ++v) {
      const float angularOffset = interval * v;
      for (float i = 0; i < dot_count; ++i) {
        const float interval_cycle_ratio = fmod(i / dot_count + cycle_ratio, 1);

        const float x1 = cos(interval_cycle_ratio * PI * 2 - PI / 2 + angularOffset) * interval_cycle_ratio * RADIUS;
        const float y1 = sin(interval_cycle_ratio * PI * 2 - PI / 2 + angularOffset) * interval_cycle_ratio * RADIUS;

        filledCircleRGBA(renderer, x1 + ox, y1 + oy, 3, 0, 0, 0, 255 * interval_cycle_ratio);

        if (i == 0) {
          const float x2 = cos(interval_cycle_ratio * PI * 2 - PI / 2 + angularOffset) * interval_cycle_ratio * RADIUS;
          const float y2 = sin(interval_cycle_ratio * PI * 2 - PI / 2 + angularOffset) * interval_cycle_ratio * RADIUS;
          lineRGBA(renderer, x2 + ox, y2 + oy, xCoords[v], yCoords[v], 0, 0, 0, 255);
        }
      }
    }

    for (float i = 0; i < dot_count; ++i) {
      const float interval_cycle_ratio = fmod(i / dot_count + cycle_ratio, 1);

      const float x1 = cos(interval_cycle_ratio * PI * 2 - PI / 2) * RADIUS;
      const float y1 = sin(interval_cycle_ratio * PI * 2 - PI / 2) * RADIUS;

      filledCircleRGBA(renderer, x1 + ox, y1 + oy, 3, 0, 0, 0, 255);
    }
  }

private:
};

int main(int, char**){
  Loop5 loop;
  loop.Run();

  return 0;
}
