#include <iostream>
#include <SDL.h>
#include "demoloop_opengl.h"
#include "graphics/2d_primitives.h"
#include "helpers.h"
#include "hsl.h"
using namespace std;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 3;

class Loop2 : public Demoloop::DemoloopOpenGL {
public:
  Loop2() : Demoloop::DemoloopOpenGL(150, 150, 150) {}

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    int ox = width / 2, oy = height / 2;

    const int num_vertices = 20;
    const float interval = (PI * 2) / num_vertices;
    float xCoords[num_vertices + 1];
    float yCoords[num_vertices + 1];
    for (int i = 0; i < num_vertices + 1; ++i) {
      const float t = i % num_vertices;
      const float interval_cycle_ratio = fmod(t / num_vertices + cycle_ratio, 1);
      const float num_extra_vertices = pow(sin(cycle_ratio * PI), 2) * 2 + 2;
      const float vertex_cycle = pow(sin(interval_cycle_ratio * PI * num_extra_vertices), 2);

      xCoords[i] = cos(interval * t) * (vertex_cycle) * RADIUS + ox;
      yCoords[i] = sin(interval * t) * (vertex_cycle) * RADIUS + oy;
    }

    for (int i = 0; i < num_vertices; ++i) {
      const float t = i;
      const float interval_cycle_ratio = fmod(t / num_vertices + cycle_ratio, 1);
      const auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);
      setColor(color);
      triangle(gl, ox, oy, xCoords[i], yCoords[i], xCoords[i + 1], yCoords[i + 1]);
    }
  }

private:
};

int main(int, char**){
  Loop2 loop;
  loop.Run();

  return 0;
}
