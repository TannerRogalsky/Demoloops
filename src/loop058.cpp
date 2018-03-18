#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec2 even(vec2 x) {
  return (mod(x, 2.0) - 0.5) * 2.0;
}

vec2 movingTiles(vec2 _st, float _layers, float _speed){
  float time = pow(sin(cycle_ratio*DEMOLOOP_M_PI), 2.0) / 2.0;
  vec2 grid = floor(fract(_st) * _layers);
  _st += even(grid.yx) * time;
  return fract(_st);
}

float circle(vec2 _st, float _radius){
  return smoothstep(_radius,_radius+0.01,length(0.5 - _st));
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  tc.x *= demoloop_ScreenSize.x/demoloop_ScreenSize.y;

  tc = movingTiles(tc*2.,9.,0.8);

  return vec4(vec3(circle(tc, .3)),1.0);
}
#endif
)===";
class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(CYCLE_LENGTH, 1280, 720, 150, 150, 150), shader({shaderCode, shaderCode}) {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    shader.attach();
    shader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    renderTexture(gl.getDefaultTexture(), 0, 0, width, height);

    shader.detach();
  }

private:
  Shader shader;
};

int main(int, char**){
  Loop050 test;
  test.Run();

  return 0;
}
