#include <iostream>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "demoloop.h"
#include "helpers.h"
#include "hsl.h"
using namespace std;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 3;

class Loop2 : public Demoloop {
public:
  Loop2() : Demoloop(150, 150, 150) {}

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    float aspect_ratio = (width + 0.0) / height;
    int ox = width / 2, oy = height / 2;

    const int num_vertices = 20;
    const float interval = (PI * 2) / num_vertices;
    int16_t xCoords[num_vertices];
    int16_t yCoords[num_vertices];
    for (int i = 0; i < num_vertices; ++i) {
      float t = i;
      float interval_cycle_ratio = fmod(t / num_vertices + cycle_ratio, 1);

      xCoords[i] = cos(interval * t) * (interval_cycle_ratio) * RADIUS + ox;
      yCoords[i] = sin(interval * t) * (interval_cycle_ratio) * RADIUS + oy;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    filledPolygonColor(renderer, xCoords, yCoords, num_vertices, rgb2uint32(color));
  }

private:
};

int main(int, char**){
  Loop2 loop;
  loop.Run();

  return 0;
}
