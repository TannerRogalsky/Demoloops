#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;

const uint32_t CYCLE_LENGTH = 5;

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
#define PI 3.14159265359
#define TWO_PI 6.28318530718

vec2 rotate(vec2 pos, float rad){
    return mat2(vec2(cos(rad), -sin(rad)), vec2(sin(rad), cos(rad))) * pos;
}

float colorStrength(float dist){
    return 1.0 - smoothstep(dist, dist + 0.006, 0.5);
}

float shape(vec2 diff, float sides){
    float a = atan(diff.x,diff.y);
    float r = TWO_PI/sides;
    float d = cos(floor(.5+a/r)*r-a)*(length(diff) / cos(PI / sides));
    return d;
}

vec4 effect(vec4 unused1, Image texture, vec2 tc, vec2 screen_coords) {
  tc = tc*2.-1.;
  tc.x *= demoloop_ScreenSize.x/demoloop_ScreenSize.y;
  tc = rotate(tc, sin(cycle_ratio * TWO_PI) * length(tc) * 2.0 + PI);


  float sides = 3.5;
  float d = shape(tc, sides);

  vec3 baseColor = vec3(0.5 + 0.5 * cos(cycle_ratio * TWO_PI),
                  0.5 + 0.5 * cos(cycle_ratio * TWO_PI + PI),
                  0.5 + 0.5 * sin(cycle_ratio * TWO_PI));

  float c = colorStrength(d);
  vec3 color = vec3(pow(max(min(c, 1.-d), 0.0), 1.5)) * baseColor;
  return vec4(color,1.0);
}
#endif
)===";

const static std::string headShaderCode = R"===(
uniform mediump float cycle_ratio;

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
#define PI 3.14159265359
#define TWO_PI 6.28318530718

#define LINE_THICKNESS 0.0025
#define LINE_SHINE 0.006

vec4 effect(vec4 unused1, Image texture, vec2 tc, vec2 screen_coords) {
  tc = tc*2.-1.;

  vec3 baseColor = vec3(0.5 + 0.5 * cos(cycle_ratio * TWO_PI),
                  0.5 + 0.5 * cos(cycle_ratio * TWO_PI + PI),
                  0.5 + 0.5 * sin(cycle_ratio * TWO_PI));

  float d = length(tc);
  float c = smoothstep(0.5, 0.51, d);
  vec3 color = c * 0.75 * baseColor;
  return vec4(color, 1.-d + 1.-c);
}
#endif
)===";

class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(CYCLE_LENGTH, 480 * 2, 480 * 2, 150, 150, 150), shader({shaderCode, shaderCode}), headShader({headShaderCode, headShaderCode}) {
    glDisable(GL_DEPTH_TEST);
    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    shader.attach();
    shader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    renderTexture(gl.getDefaultTexture(), -width / 2.0, -height / 2.0, width, height);

    shader.detach();

    glm::vec2 headPos(0, -height * 0.3);
    headPos = glm::rotate(headPos, sinf(cycle_ratio * DEMOLOOP_M_PI * 2.0));

    headShader.attach();
    headShader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    renderTexture(gl.getDefaultTexture(), headPos.x - 200 / 2, headPos.y - 200 / 2, 200, 200);

    headShader.detach();
  }

private:
  Shader shader;
  Shader headShader;
};

int main(int, char**){
  Loop050 test;
  test.Run();

  return 0;
}
