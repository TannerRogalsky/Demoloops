
#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;


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

#define NUM_COLORS 5
uniform vec3 gradientColors[NUM_COLORS];

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  float x = tc.x;
  float y = tc.y;

  vec2 c = vec2(
    (1.0 - pow(sin(cycle_ratio * DEMOLOOP_M_PI), 5.0)) / 28.5 + 0.35,
    0.3 + sin(cycle_ratio * DEMOLOOP_M_PI * 2.0) / 30.0
  );

  highp vec2 z;
  z.x = 3.0 * (x - 0.5);
  z.y = 2.0 * (y - 0.5);

  z /= 2.0;
  z.y -= 0.4;

  const float B = 256.0;

  int iterations = 0;
  for(int i=0; i<iter; i++) {
    z = vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;

    if(dot(z,z)>(B*B)) break;
    iterations = i;
  }

  float fIterations = float(iterations);
  float logLogDot = log2(log2(dot(z,z)));
  float q = (fIterations - logLogDot + 4.0) / float(iter);
  float stepIncrement = 1.0 / (float(NUM_COLORS) + 1.0);
  float step = 0.0;

  vec3 mixed = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < NUM_COLORS; ++i) {
    mixed = mix(mixed, gradientColors[i], smoothstep(step, step + stepIncrement, q)); step += stepIncrement;
  }
  return vec4(mixed, 1.0);
}
#endif
)===";
class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(CYCLE_LENGTH, 150, 150, 150), shader({shaderCode, shaderCode}) {
    glm::vec3 colors[5] = {
      glm::vec3(0.8, 0.6, 0.1),
      glm::vec3(0.917, 0.467, 0.008),
      glm::vec3(1.0, 0.0, 0.0),
      glm::vec3(1.0, 0.7, 0.0),
      glm::vec3(0.0, 0.0, 0.0),
    };
    shader.sendFloat("gradientColors", 3, &colors[0].x, 5);
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
