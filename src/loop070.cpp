
#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 2;

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
#define PI DEMOLOOP_M_PI

float map(vec3 p) {
  float g = 0.25;
  vec3 q = (fract(p/g) * 2.0 - 1.0) * g;
  vec3 s = 0.5 - abs(normalize(q));
  q = sign(q) * s;
  float md = length(q) - 0.5;
  float tr = mix(1.0, 2.0, pow(sin(p.z*PI) * sin((md + 0.65) * PI), 2.0));
  float cv = pow(length(p.xy), 0.5) - tr;
  md = max(md, -cv);
  return md;
}

vec4 effect(vec4 globalColor, Image texture, vec2 tc, vec2 screen_coords) {
  vec2 uv = tc;
  uv = uv * 2.0 - 1.0;
  uv.x *= demoloop_ScreenSize.x / demoloop_ScreenSize.y;

  vec3 r = normalize(vec3(uv, 1.0));
  vec3 o = vec3(0.0, 0.0, cycle_ratio);

  float fd = map(o + r);

  vec3 sc = vec3(dot(uv, uv), 0.5, 1.0);
  vec3 ec = vec3(1.0, 1.0, 1.0);
  vec3 fc = mix(sc, ec, 1.0 - fd);

  return vec4(sqrt(fc),1.0);
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
