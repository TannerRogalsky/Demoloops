
#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;


const uint32_t CYCLE_LENGTH = 3;

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec2 rotate(vec2 pos, float rad){
    return pos * mat2(cos(rad), -sin(rad), sin(rad), cos(rad));
}

float random (in vec2 st) {
  highp float p = dot(st.xy, vec2(12.9898,78.233));
  return fract(sin(p)*4358.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    // u = mix(f, u, pow(sin(u_time * 2.0), 2.0));

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitud = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitud * noise(st);
        st *= 2.;
        amplitud *= .5;
    }
    return value;
}

vec4 effect(vec4 globalColor, Image texture, vec2 tc, vec2 screen_coords) {
  float cycle = cycle_ratio;
  vec3 color = vec3(0.0);

  vec2 p = cos((tc*2.0-1.0) * DEMOLOOP_M_PI) * 4.0;
  vec2 q = vec2(fbm(p), fbm(p + vec2(5.2,1.3)));

  q = rotate(q, cycle * DEMOLOOP_M_PI * 2.0);

  vec2 r = vec2(fbm( p + 4.0*q + vec2(1.7,9.2) ),
                fbm( p + 4.0*q + vec2(8.3,2.8) ) );


  color.r += fbm(p + 3.5*r) * 1.4;
  color.g += fbm(p + 1.5*r) * 1.4;
  color.b += fbm(p + 0.5*r) * 1.4;

  return vec4(color, 1.0);
}
#endif
)===";
class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(CYCLE_LENGTH, 720, 720, 150, 150, 150), shader({shaderCode, shaderCode}) {
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
