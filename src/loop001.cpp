#include <iostream>
#include <SDL.h>
#include "demoloop_opengl.h"
#include "graphics/2d_primitives.h"
#include "helpers.h"
#include "hsl.h"
using namespace std;
using namespace Demoloop;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 3;

class Loop1 : public DemoloopOpenGL {
public:
  Loop1() : DemoloopOpenGL(150, 150, 150) {
  }

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    float count = 50;
    float aspect_ratio = (width + 0.0) / height;
    int ox = width / 2, oy = height / 2;

    auto colorLocation = Shader::defaultShader->getAttribLocation("ConstantColor");
    for (int i = 0; i < count; ++i) {
      float interval_cycle_ratio = fmod(i / count + cycle_ratio, 1);
      auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);

      float t = -interval_cycle_ratio;
      int x1 = cos(t * PI * 2) * sin(i * PI * 2) * aspect_ratio * RADIUS + ox;
      int y1 = sin(t * PI * 2) * RADIUS + oy;
      glVertexAttrib4f(colorLocation, color.r / 255.0, color.g / 255.0, color.b / 255.0, 1);
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
