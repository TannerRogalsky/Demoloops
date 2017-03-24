#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/3d_primitives.h"
#include "graphics/shader.h"
#include "graphics/canvas.h"
#include "graphics/mesh.h"
#include "graphics/image.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

const static std::string sphereShaderCode = R"===(
varying vec4 VaryingVertexPosition;

uniform mediump float cycle_ratio;
uniform mediump float offset;

#define NOISE pattern
#define NUM_NOISE_OCTAVES 3
#define DEMOLOOP_M_PI 3.1459

float hash(float n) { return fract(sin(n) * 1e4 * offset); }

float noise(vec3 x) {
    const vec3 step = vec3(110, 241, 171);

    vec3 i = floor(x);
    vec3 f = fract(x);

    // For performance, compute the base input to a 1D hash from the integer part of the argument and the
    // incremental change to the 1D based on the 3D -> 1D wrapping
    float n = dot(i, step);

    vec3 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix( hash(n + dot(step, vec3(0, 0, 0))), hash(n + dot(step, vec3(1, 0, 0))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 0))), hash(n + dot(step, vec3(1, 1, 0))), u.x), u.y),
               mix(mix( hash(n + dot(step, vec3(0, 0, 1))), hash(n + dot(step, vec3(1, 0, 1))), u.x),
                   mix( hash(n + dot(step, vec3(0, 1, 1))), hash(n + dot(step, vec3(1, 1, 1))), u.x), u.y), u.z);
}

float fbm(vec3 x) {
  float v = 0.0;
  float a = 0.5;
  vec3 shift = vec3(100);
  for (int i = 0; i < NUM_NOISE_OCTAVES; ++i) {
    v += a * noise(x);
    x = x * 2.0 + shift;
    a *= 0.5;
  }
  return v;
}

float pattern( in vec3 p ) {
  vec3 q = vec3( fbm( p + vec3(0.0, 0.0, 1.3) ),
                 fbm( p + vec3(5.2, 1.3, 0.0) ),
                 fbm( p + vec3(4.1, 3.7, 2.0) ) );

  vec3 r = vec3( fbm( p + 4.0*q + vec3(1.7, 9.2, 0.0) ),
                 fbm( p + 4.0*q + vec3(8.3, 2.8, 0.0) ),
                 fbm( p + 4.0*q + vec3(0.0, 0.0, 0.0) ) );

  return fbm( p + 4.0*r * pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2.0) );
}

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  VaryingVertexPosition = vertpos;

  float f = NOISE(VaryingVertexPosition.xyz);
  vertpos.xyz *= 1.0 + f;
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float f = NOISE(VaryingVertexPosition.xyz);

  vec3 mixed = vec3(0.015, 0.0, 0.505);
  mixed = mix(mixed, vec3(0.0, 0.3686, 1.0) * (f + 0.5), smoothstep(0.0, 0.25, f));
  mixed = mix(mixed, vec3(0.0941, 0.741, 0.172) * (f + 0.5), smoothstep(0.25, 0.48, f));
  mixed = mix(mixed, vec3(0.553, 0.494, 0.494) * (f + 0.5), smoothstep(0.48, 0.485, f));
  mixed = mix(mixed, vec3(f), smoothstep(0.485, 1.0, f));

  return Texel(texture, tc) * color * vec4(mixed, 1.0);
}
#endif
)===";

class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(720, 720, 0, 0, 0),
        sphereShader({sphereShaderCode, sphereShaderCode}),
        sphereMesh(sphere(1, 128, 128)),
        offset(static_cast<float>(rand()) / RAND_MAX)  {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    sphereShader.sendFloat("offset", 1, &offset, 1);
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    const float scale = 10;

    glm::mat4 transform;
    transform = glm::translate(transform, {0, 0, 50});
    transform = glm::scale(transform, {scale, scale, scale});
    transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0.3, 1, 0});

    sphereShader.attach();
    sphereShader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);
    sphereMesh.draw(transform);
    sphereShader.detach();
  }

private:
  Shader sphereShader;
  Mesh sphereMesh;
  const float offset;
};

int main(int, char**){
  srand(time(0)); rand();

  Loop050 test;
  test.Run();

  return 0;
}
