
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

vec2 cadd( vec2 a, float s ) { return vec2( a.x+s, a.y ); }
vec2 cmul( vec2 a, vec2 b )  { return vec2( a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x ); }
vec2 cdiv( vec2 a, vec2 b )  { float d = dot(b,b); return vec2( dot(a,b), a.y*b.x - a.x*b.y ) / d; }
vec2 cpow( vec2 z, float n ) { float r = length( z ); float a = atan( z.y, z.x ); return pow( r, n )*vec2( cos(a*n), sin(a*n) ); }
vec2 csqrt( vec2 z )         { float m = length(z); return sqrt( 0.5*vec2(m+z.x, m-z.x) ) * vec2( 1.0, sign(z.y) ); }
vec2 cconj( vec2 z )         { return vec2(z.x,-z.y); }

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  float x = tc.x;
  float y = tc.y;

  vec2 c;
  c.x = 2.0*(x - 0.5)*demoloop_ScreenSize.x/demoloop_ScreenSize.y;
  c.y = 2.0*(y - 0.5);

  highp vec2 z = c;
  // z.x += pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2.0);

  const float B = 256.0;

  int iterations = 0;
  for(int i=0; i<iter; i++) {
    // float x = (z.x * z.x - z.y * z.y) + c.x;
    // float y = (z.y * z.x + z.x * z.y) + c.y;
    // z = vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
    z = cpow(z, 2.0 + cycle_ratio * 6.0) + c;

    iterations = i;
    if(dot(z,z)>(B*B)) break;
    // if ((x * x + y * y) > 256.0) break;
    // z.x = x;
    // z.y = y;
  }

  float q = (iterations == iter ? 0.0 : float(iterations)) / float(iter);
  // float q = (i - log2(log2(dot(z,z))) + 4.0) / iter;
  vec3 mixed = mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), q);
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
