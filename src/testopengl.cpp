#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "demoloop_opengl.h"
#include "opengl_helpers.h"
#include "hsl.h"
using namespace std;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 3;

class Test4 : public DemoloopOpenGL {
public:
  Test4() : DemoloopOpenGL(150, 150, 150) {
    std::cout << glGetString(GL_VERSION) << std::endl;

    const char* vertexShader = "vec4 position(mat4 transform_proj, vec4 vertpos) { return transform_proj * vertpos; }";
    const char* fragShader = "vec4 effect(mediump vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) { return Texel(tex, texcoord) * vcolor; }";
    std::string v = createVertexCode(vertexShader);
    std::string f = createFragmentCode(fragShader);
    GLuint program = loadProgram(v.c_str(), f.c_str());
    cout << program << endl;
  }

  void Update(float dt) {
    t += dt;
    // cout << t << endl;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    float count = 50;
    float aspect_ratio = (width + 0.0) / height;
    int ox = width / 2, oy = height / 2;

    for (int i = 0; i < count; ++i) {
      float interval_cycle_ratio = fmod(i / count + cycle_ratio, 1);
      auto color = hsl2rgb(interval_cycle_ratio, 1, 0.5);

      float t = -interval_cycle_ratio;
      int x1 = cos(t * PI * 2) * sin(i * PI * 2) * aspect_ratio * RADIUS + ox;
      int y1 = sin(t * PI * 2) * RADIUS + oy;
      filledCircleRGBA(renderer, x1, y1, 2, color.r, color.g, color.b, 255);
    }
  }

private:
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
