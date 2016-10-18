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

class Loop3 : public Demoloop {
public:
  Loop3() : Demoloop(150, 150, 150) {}

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
      // float interval_cycle_ratio = fmod(t / num_vertices + cycle_ratio, 1);

      xCoords[i] = cos(interval * t - PI / 10) * RADIUS + ox;
      yCoords[i] = sin(interval * t - PI / 10) * RADIUS + oy;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    filledPolygonColor(renderer, xCoords, yCoords, num_vertices, rgb2uint32(color));

    const int dot_count = 20;
    for (float i = 0; i < dot_count; ++i) {
      float interval_cycle_ratio = fmod(i / dot_count + cycle_ratio, 1);

      float x1 = cos(cycle_ratio * PI * 2) * RADIUS;
      float y1 = sin(cycle_ratio * PI * 2) / num_vertices * RADIUS;

      float c = cos(interval_cycle_ratio * PI * 2);
      float s = sin(interval_cycle_ratio * PI * 2);

      x1 = c * x1 - s * y1;
      y1 = s * x1 + c * y1;
      filledCircleRGBA(renderer, x1 + ox, y1 + oy, 3, 0, 0, 0, 255);
    }
  }

private:
};

int main(int, char**){
  Loop3 loop;
  loop.Run();

  return 0;
}