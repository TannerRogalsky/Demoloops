
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
const int iter = 100;

vec3 mix(float a, vec3 v1, vec3 v2) { return v1 * (1.0 - a) + v2 * a; }

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  float x = tc.x;
  float y = tc.y;

  vec2 c;
  c.x = 2.0*(x - 0.5)*demoloop_ScreenSize.x/demoloop_ScreenSize.y - 0.75;
  c.y = 2.0*(y - 0.5);

  vec2 z = c;
  // z.x += pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2.0);

  int i;
  for(i=0; i<iter; i++) {
      float x = (z.x * z.x - z.y * z.y) + c.x;
      float y = (z.y * z.x + z.x * z.y) + c.y;

      if ((x * x + y * y) > 12.0) break;
      z.x = x;
      z.y = y;
  }

  float q = (i == iter ? 0.0 : float(i)) / 100.0;
  vec3 mixed = mix(q, vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0));
  return vec4(mixed, 1.0);
}
#endif
)===";
class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(150, 150, 150), shader({shaderCode, shaderCode}) {
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
