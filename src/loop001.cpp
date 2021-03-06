
#include <SDL.h>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "helpers.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;


const float PI = 3.1459;
const uint32_t CYCLE_LENGTH = 3;

class Loop1 : public Demoloop {
public:
  Loop1() : Demoloop(CYCLE_LENGTH, 150, 150, 150) {
  }

  void Update() {
    const float RADIUS = height / 3;


    const float cycle_ratio = getCycleRatio();
    float count = 50;
    float aspect_ratio = (width + 0.0) / height;
    int ox = width / 2, oy = height / 2;

    for (int i = 0; i < count; ++i) {
      float interval_cycle_ratio = fmod(i / count + cycle_ratio, 1);
      auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);

      float t = -interval_cycle_ratio;
      int x1 = cos(t * PI * 2) * sin(i * PI * 2) * aspect_ratio * RADIUS + ox;
      int y1 = sin(t * PI * 2) * RADIUS + oy;

      setColor(color);
      circle(gl, x1, y1, 3);
    }
  }

private:
};

int main(int, char**){
  Loop1 loop;
  loop.Run();

  return 0;
}
