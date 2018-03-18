
#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;


const uint32_t CYCLE_LENGTH = 10;

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
highp vec3 mod289(highp vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

highp vec2 mod289(highp vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

highp vec3 permute(highp vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(  vec2 v)
  {
  const highp vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  highp vec2 i  = floor(v + dot(v, C.yy) );
  highp vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  highp vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  highp vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  highp vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
    + i.x + vec3(0.0, i1.x, 1.0 ));

  highp vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  highp vec3 x = 2.0 * fract(p * C.www) - 1.0;
  highp vec3 h = abs(x) - 0.5;
  highp vec3 ox = floor(x + 0.5);
  highp vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  highp vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

float snoise(float x, float y) {
  return snoise(vec2(x, y));
}

float noise(float x, float y, float a, float b) {
  return snoise(x * a / 256.0, y * b / 256.0);
}

vec3 mix(float a, vec3 v1, vec3 v2) { return v1 * (1.0 - a) + v2 * a; }

float smoothStep(float w, float a, float b) {
  if (w>=b) return 1.0;
  if (w<=a) return 0.0;
  float d = b-a;
  return (w-a)/d;
}

vec3 colsca(vec3 c, float s) {
  c.r*=s;
  c.g*=s;
  c.b*=s;
  if (c.r>1.0) c.r=1.0;
  if (c.g>1.0) c.g=1.0;
  if (c.b>1.0) c.b=1.0;

  return c;
}

vec3 circle(float x, float y, float size) {
  return vec3(mix(sqrt(x*x + y*y) / size, vec3(0, 0, 0), vec3(1, 1, 1)));
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  float x = tc.x;
  float y = tc.y;

  float rx = 2.0*(x - 0.5)*demoloop_ScreenSize.x/demoloop_ScreenSize.y;
  float ry = 2.0*(y - 0.5);

  float size = 0.5;
  float circle1 = smoothStep(sqrt(rx*rx + ry*ry), size, size + 0.001);
  rx += 0.6;
  float circle2 = smoothStep(sqrt(rx*rx + ry*ry), size, size + 0.001);

  // float f = mix(r, circle1, circle2);
  float f = (circle2 + circle1) / 2;
  // f = floor(f);

  // vec3 c = vec3(mix(f, vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 1.0)));
  // vec3 c = vec3(f, f, f);
  // vec3 c = vec3((circle2 + circle1) / 2, (circle2 + circle1) / 2, (circle2 + circle1) / 2);



  // vec3 c1 = circle(rx, ry, 0.5);
  // vec3 c2 = circle(rx + 0.6, ry, 0.5);

  // vec3 c = mix(0.5, c1, c2);
  // c -= 0.5;

  // return vec4(c, 1.0);


  float v = 0.0;
  for (int i = 0; i < 3; i++) {
      vec3 mb = vec3(float(i) / 3.0, 0.5, 0.1);
      mb.x *= demoloop_ScreenSize.x / demoloop_ScreenSize.y;
      float dx = mb.x - x;
      float dy = mb.y - y;
      float r = mb.z;
      v += r*r/(dx*dx + dy*dy);
  }
  if (v > 1.0) {
      return vec4(x, y, 0.0, 1.0);
  } else {
      return vec4(0.0, 0.0, 0.0, 1.0);
  }
}
#endif
)===";
class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(CYCLE_LENGTH, 150, 150, 150), shader({shaderCode, shaderCode}) {
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
