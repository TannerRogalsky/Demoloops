#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;

const Vertex quad[4] = {
  {-0.5, 0.5, 1, 0, 1},
  {-0.5, -0.5, 1, 0, 0},
  {0.5, -0.5, 1, 1, 0},
  {0.5, 0.5, 1, 1, 1}
};
const glm::vec3 twoDAxis = {0, 0, 1};

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
#define LINE_THICKNESS 0.025
#define LINE_SHINE 0.04

float colorStrength(float dist){
    return 1.0 - smoothstep(LINE_THICKNESS - LINE_SHINE,
                        LINE_THICKNESS + LINE_SHINE,
                        dist);
}

float shape(vec2 diff, float radius){
    float a = atan(diff.x,diff.y);
    float r = DEMOLOOP_M_PI * 2.0 / 3.0; // 3 sides
    float d = cos(floor(.5+a/r)*r-a)*(length(diff));
    return abs(d - radius);
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  tc = tc*2.-1.;
  // tc.x *= demoloop_ScreenSize.x/demoloop_ScreenSize.y;

  float c = colorStrength(shape(tc, .4));
  return vec4(c) * color;
}
#endif
)===";

const uint32_t CYCLE_LENGTH = 7;
class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(CYCLE_LENGTH, 720, 720, 0, 0, 0), RADIUS(height / 5), shader({shaderCode, shaderCode}) {
    glDisable(GL_DEPTH_TEST);
    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();
    const float PI = static_cast<float>(DEMOLOOP_M_PI);

    shader.attach();
    shader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    for (uint32_t i = 0; i < NB_REPEAT; ++i) {
      float rep = static_cast<float>(i);
      float trail = rep * TRAIL_DURATION/NB_REPEAT;
      float time = cycle_ratio * DEMOLOOP_M_PI * 2 - trail;
      for (uint32_t j = 0; j < NUM_SHAPES; ++j) {
        float shape = static_cast<float>(j) / NUM_SHAPES;

        glm::vec2 pos(0, 0);
        pos.x += cosf(time);
        pos.x *= sinf(time);

        pos.y += sinf(time);

        pos *= height / 3;

        glm::mat4 m = glm::mat4();
        // m = glm::rotate(m,  -cycle_ratio * PI * 2, twoDAxis);
        m = glm::rotate(m, PI * 2 * shape, twoDAxis);
        m = glm::translate(m, {pos.x, pos.y, 0});
        m = glm::rotate(m, cycle_ratio * PI * 2, twoDAxis);
        m = glm::scale(m, {RADIUS * 0.6 + glm::length(pos) * 0.4, RADIUS * 0.6 + glm::length(pos) * 0.4, 1});

        glm::vec3 color = glm::vec3(0.5 + 0.5 * cos(shape * 5 + cycle_ratio * PI * 2),
                                   0.5 + 0.5 * cos(shape * 5 + cycle_ratio * PI * 2 + 3.14),
                                   0.5 + 0.5 * sin(shape * 5 + cycle_ratio * PI * 2));
        color = glm::pow(color, glm::vec3(1.0/2.2));
        color *= glm::vec3(255, 255, 255);

        // setColor(hsl2rgb(fmod(shape + cycle_ratio, 1), 1, 0.5), powf(1.0 - rep / NB_REPEAT, 2.0) * 255);
        setColor(color.x, color.y, color.z, powf(1.0 - rep / NB_REPEAT, 2.0) * 255);
        gl.triangleFan(quad, 4, m);
      }
    }

    shader.detach();
  }

private:
  const float RADIUS;
  Shader shader;

  const uint32_t NUM_SHAPES = 5;
  const uint32_t NB_REPEAT = 16;
  const float TRAIL_DURATION = 1.6;
};

int main(int, char**){
  Loop050 test;
  test.Run();

  return 0;
}
