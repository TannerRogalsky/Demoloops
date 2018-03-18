
#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;


const float CYCLE_LENGTH = 10;

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.14159265359
#define DEMOLOOP_TWO_PI 6.28318530718

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float polygon(vec2 st, float numVertices) {
  // st.x += cos(cycle_ratio * DEMOLOOP_TWO_PI);
  // st.y += sin(cycle_ratio * DEMOLOOP_TWO_PI);

  // Angle and radius from the current pixel
  // float a = atan(st.x - cos(cycle_ratio * DEMOLOOP_TWO_PI) * 2.0, st.y + sin(cycle_ratio * DEMOLOOP_TWO_PI) * 2.0)+DEMOLOOP_M_PI;
  float r = DEMOLOOP_TWO_PI / float(numVertices);
  float a = atan(st.x, st.y) + DEMOLOOP_M_PI + cycle_ratio * r;

  // Shaping function that modulate the distance
  // float d = cos(floor(a/r)*r-a)*length(st);
  float c = 0.0;
  float d = cos( (fract(a/r)-c)*r) * length(st);

  return d;
}

vec4 effect(vec4 color, Image texture, vec2 st, vec2 screen_coords) {
  float t = cycle_ratio;
  vec3 c = vec3(0.0);
  float d = 0.0;

  // Remap the space to -1. to 1.
  st = st * 2. - 1.;
  st.x *= demoloop_ScreenSize.x/demoloop_ScreenSize.y;

  d = polygon(st, 5.0);
  float f = mod(fract(d * 7.0) + cycle_ratio * 2.0, 1.0);
  d = smoothstep(0.0, f, d);

  // c = vec3(d);
  c = hsv2rgb(vec3(mod(d + t + (1.0 - length(st) / 4.0), 1.0), 1.0, 0.9));
  c = mix(c, vec3(0, 0, 0), dot(st, st) * 0.15);

  return vec4(c,1.0);
}
#endif
)===";

class Geometric : public Demoloop {
public:
  Geometric() : Demoloop(CYCLE_LENGTH, 150, 150, 150), shader({shaderCode, shaderCode}) {
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
  Geometric test;
  test.Run();

  return 0;
}
