#include <iostream>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "demoloop.h"
#include "helpers.h"
#include "math_helpers.h"
#include "hsl.h"
using namespace std;

#define MAX_VERTS 9

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

    const int num_vertices = cycle_ratio * 6 + 3;
    // const int num_vertices = 3;
    const float rotation_offset = rotationOffset(num_vertices);

    const float interval = (PI * 2) / num_vertices;
    int16_t xCoords[MAX_VERTS];
    int16_t yCoords[MAX_VERTS];
    for (int i = 0; i < num_vertices; ++i) {
      float t = i;
      xCoords[i] = cos(interval * t - rotation_offset) * RADIUS + ox;
      yCoords[i] = sin(interval * t - rotation_offset) * RADIUS + oy;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    filledPolygonColor(renderer, xCoords, yCoords, num_vertices, rgb2uint32(color));

    const int dot_count = 50;
    for (float i = 0; i < dot_count; ++i) {
      float interval_cycle_ratio = fmod(i / dot_count + cycle_ratio, 1);

      const int INTERNAL_RADIUS = cos(PI / num_vertices) * RADIUS;

      float x1 = cos(interval_cycle_ratio * PI * 2) * INTERNAL_RADIUS;
      float y1 = sin(interval_cycle_ratio * PI * 2) * INTERNAL_RADIUS;
      x1 += sin(interval_cycle_ratio * PI * 2 * (num_vertices - 1)) * INTERNAL_RADIUS * 0.5;
      y1 += cos(interval_cycle_ratio * PI * 2 * (num_vertices - 1)) * INTERNAL_RADIUS * 0.5;

      float c = cos(rotation_offset);
      float s = sin(rotation_offset);
      float x2 = c * x1 - s * y1;
      float y2 = s * x1 + c * y1;

      float color = interval_cycle_ratio * 255;
      filledCircleRGBA(renderer, x2 + ox, y2 + oy, 3, color, color, color, 255);
    }
  }

private:
};

int main(int, char**){
  Loop6 loop;
  loop.Run();

  return 0;
}
