
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 10;

class Loop6 : public Demoloop {
public:
  Loop6() : Demoloop(150, 150, 150) {
    glDisable(GL_DEPTH_TEST);
  }

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    int ox = width / 2, oy = height / 2;

    const int num_vertices = 5;
    const float interval = (PI * 2) / num_vertices;
    float xCoords[num_vertices];
    float yCoords[num_vertices];
    for (int i = 0; i < num_vertices; ++i) {
      float t = i;
      xCoords[i] = cos(interval * t - PI / 10) * RADIUS + ox;
      yCoords[i] = sin(interval * t - PI / 10) * RADIUS + oy;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    setColor(color);
    polygon(gl, xCoords, yCoords, num_vertices);

    const int dot_count = 20;
    for (float i = 0; i < dot_count; ++i) {
      float interval_cycle_ratio = fmod(i / dot_count + cycle_ratio, 1);

      float x1 = cos(interval_cycle_ratio * PI * 2) * RADIUS / 2;
      float y1 = sin(interval_cycle_ratio * PI * 2) * RADIUS / 2;
      x1 += cos(interval_cycle_ratio * PI * 2 * (num_vertices + 1) + PI / 2) * RADIUS / 4;
      y1 += sin(interval_cycle_ratio * PI * 2 * (num_vertices + 1) + PI / 2) * RADIUS / 4;

      float next_interval_cycle_ratio = fmod(fmod(i + 1, dot_count) / dot_count + cycle_ratio, 1);
      float x2 = cos(next_interval_cycle_ratio * PI * 2) * RADIUS / 2;
      float y2 = sin(next_interval_cycle_ratio * PI * 2) * RADIUS / 2;
      x2 += cos(next_interval_cycle_ratio * PI * 2 * (num_vertices + 1) + PI / 2) * RADIUS / 4;
      y2 += sin(next_interval_cycle_ratio * PI * 2 * (num_vertices + 1) + PI / 2) * RADIUS / 4;

      setColor(255, 255, 255, (cos(cycle_ratio * PI * 2) + 1) / 2 * 255);
      line(gl, x1 + ox, y1 + oy, x2 + ox, y2 + oy);

      setColor(0, 0, 0);
      circle(gl, x1 + ox, y1 + oy, 3);
    }
  }

private:
};

int main(int, char**){
  Loop6 loop;
  loop.Run();

  return 0;
}
