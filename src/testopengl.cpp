#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "common/matrix.h"
#include "demoloop_opengl.h"
#include "opengl_helpers.h"
#include "graphics/gl.h"
#include "hsl.h"
using namespace std;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 3;

struct Vector3f
{
  float x, y, z;
};

class Test4 : public DemoloopOpenGL {
public:
  Test4() : DemoloopOpenGL(150, 150, 150) {
    std::cout << glGetString(GL_VERSION) << std::endl;
    gl.setViewport({0, 0, width, height});
  }

  void Update(float dt) {
    t += dt;

    // const float RADIUS = height / 3;

    // float cycle = fmod(t, CYCLE_LENGTH);
    // float cycle_ratio = cycle / CYCLE_LENGTH;
    // float count = 50;
    // float aspect_ratio = (width + 0.0) / height;

    // for (int i = 0; i < count; ++i) {
    //   float interval_cycle_ratio = fmod(i / count + cycle_ratio, 1);
    //   auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);

    //   float t = -interval_cycle_ratio;
    //   int x1 = cos(t * PI * 2) * sin(i * PI * 2) * aspect_ratio * RADIUS;
    //   int y1 = sin(t * PI * 2) * RADIUS;
    //   filledCircleRGBA(renderer, x1, y1, 2, color.r, color.g, color.b, 255);
    // }

    Demoloop::Vertex coords[4];
    coords[0].x = -100 + 200;
    coords[0].y = -100 + 200;
    coords[1].x = 100 + 200;
    coords[1].y = -100 + 200;
    coords[2].x = 100 + 200;
    coords[2].y = 100 + 200;
    coords[3].x = -100 + 200;
    coords[3].y = 100 + 200;
    gl.polygon(coords, 4);
  }

private:
  Demoloop::GL gl;
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
