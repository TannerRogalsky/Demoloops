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
#define PI 3.14159265359
#define TWO_PI 6.28318530718

vec4 h2rgb(float c) {
  return  clamp(abs( fract(c + vec4(3,2,1,0)/3.) * 6. - 3.) -1. , 0., 1.);
}

vec4 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return vec4(c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y), 1.0);
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  tc.x *= demoloop_ScreenSize.x/demoloop_ScreenSize.y;
  vec2 st = fract(tc * 4.0) * 2.0 - 1.0;

  int N = 6;

  float a = atan(st.x,st.y);
  float r = TWO_PI/float(N);
  float d = cos(floor(.5+a/r)*r-a)*length(st) - t;

  vec3 c = vec3(step(0.1, mod(d - t, 0.2)));
  return vec4(c,1.0) * hsv2rgb(vec3(fract(d - t), 1.0, 1.0));
  // return step(.1, mod(d,.2)) * h2rgb( d );
  // return step(.1, mod(d,.2)) *  hsv2rgb(vec3(fract(d - t), 1.0, 1.0));
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
