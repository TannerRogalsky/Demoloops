
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

#define DEMOLOOP_M_PI 3.14159265359
#define DEMOLOOP_TWO_PI 6.28318530718

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
float intersect(float value1, float value2)
{
    return value1 * value2;
}

float merge(float value1, float value2)
{
    return 1-((1-value1) * (1-value2));
}

float halfspace(vec2 origin, vec2 direction, float feather, vec2 point)
{
    return smoothstep(
        feather/2,
        -feather/2,
        dot(point - origin, direction));
}

vec2 rotate(vec2 v, float a)
{
    float cos_a = cos(a);
    float sin_a = sin(a);
    return vec2(
        cos_a * v.x - sin_a * v.y,
        cos_a * v.y + sin_a * v.x);
}

float square(vec2 point, float size, float feather)
{
    return
        halfspace(vec2(size/2, 0), vec2(1, 0), feather, point) *
        halfspace(vec2(0, size/2), vec2(0, 1), feather, point) *
        halfspace(vec2(-size/2, 0), vec2(-1, 0), feather, point) *
        halfspace(vec2(0, -size/2), vec2(0, -1), feather, point);
}

float circle(vec2 point, float size, float feather)
{
    return smoothstep(-feather/2, feather/2, size - length(point));
}

vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords)
{
    vec2 point = texture_coords.xy * 2 - vec2(1, 1);
    float modulated_cycle = pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2);

    vec2 axle = vec2(cos(-cycle_ratio*DEMOLOOP_TWO_PI), sin(-cycle_ratio*DEMOLOOP_TWO_PI));
    float value = merge(
      square(rotate(point - 0.2*axle, -cycle_ratio * DEMOLOOP_TWO_PI), 0.3, 0.1),
      square(rotate(point + 0.3*axle, cycle_ratio * DEMOLOOP_TWO_PI), 0.5, 0.1));
    value = fract(merge(value, circle(point, 0.8, 0.1)/2) * (modulated_cycle * 1.5 + 0.25));

    return vec4(color.xyz,
        smoothstep(0.15, 0.25, value) *
        smoothstep(0.85, 0.75, value));
}
#endif
)===";

class Geometric : public Demoloop {
public:
  Geometric() : Demoloop(500, 500, 150, 150, 150), shader({shaderCode, shaderCode}) {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    setColor(42, 132, 242);

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
