#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 7;

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

vec2 movingTiles(vec2 _st, float _layers){
  // float time = pow(sin(cycle_ratio*DEMOLOOP_M_PI), 2.0) / 2.0;
  // vec2 grid = floor(fract(_st) * _layers);
  // _st += even(grid.yx) * time;
  // return fract(_st);

  float time = cycle_ratio;
  vec2 grid = floor(fract(_st) * _layers);
  float dir = step(fract(time), 0.5);
  vec2 splitter = vec2(dir, 1.0-dir);

  _st += even(grid.yx) * splitter * time * 2.0;
  return fract(_st);
}

vec4 effect(vec4 unused, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  tc.x *= demoloop_ScreenSize.x/demoloop_ScreenSize.y;

  tc = movingTiles(tc*2.,4.);

  vec2 pos = tc*-2.+1.;
  pos.y -= 0.5;

  float r = length(pos)*3.0;
  float a = atan(pos.y,pos.x);

  float sinA = sin(a);
  float f = 2. - 2. * sinA + sinA * (sqrt(abs(cos(a))) / (sinA + 1.4));
  float c = smoothstep(f,f+0.05,r);

  // vec3 color = mix(vec3(1.0, 0.0, 0.3), vec3(1.0), vec3(c));
  vec3 color = mix(vec3(1.0, 0.0, 0.3) * pow(f - r, .07), vec3(1.0), vec3(c));

  return vec4(color,1.0);
}
#endif
)===";
class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(1280, 720, 150, 150, 150), shader({shaderCode, shaderCode}) {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

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
