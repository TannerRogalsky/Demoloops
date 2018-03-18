
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
  return c * s;
}

vec3 eye(float x, float y, float b) {
  float rx = 2.0*(x-0.5)*demoloop_ScreenSize.x/demoloop_ScreenSize.y;
  float ry = 2.0*(y-0.5);
  float a = atan(ry, rx);
  float e = rx*rx + ry*ry;
  float r = sqrt(e);

  vec3 fue = vec3(1.0, 1.0, 1.0);
  vec3 den = vec3(0.3, 0.7, 0.4 + e);

  // veins
  // float ven = noise(24 * x, 24 * y, 128, 128);
  // ven = smoothStep(ven, -0.2, 0.0) - smoothStep(ven, 0.0, 0.2);
  // ven += x + pow(x, 6) * 10;
  // fue.r += 0.04 - 0.00*ven;
  // fue.g += 0.04 - 0.05*ven;
  // fue.b += 0.04 - 0.05*ven;

  // circular pattern
  float noiseOffset = pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2.0) * 1.0;
  float no = 0.8 + 0.2 * noise(4.0*r + noiseOffset, 32.0*a/DEMOLOOP_M_PI + noiseOffset, 256.0, 256.0);
  den = colsca(den, no);

  // iris
  float irisSize = 0.025 - (1.0 - pow(sin((cycle_ratio) * DEMOLOOP_M_PI), 3.0)) * 0.02;
  float f2 = smoothStep(e, irisSize, irisSize + 0.01);
  den = colsca(den, f2);

  vec3 mixed = mix(smoothStep(e, 0.35, 0.36), den, fue);

  // ring
  float ri = smoothStep(e, 0.31, 0.35) - smoothStep(e, 0.35, 0.36);
  ri = 1.0 - 0.35 * ri;
  mixed = colsca(mixed, ri);

  // reflection
  // float r3 = sqrt(r*r*r);
  // float re = noise(2.0+4.0*r3*cos(a), 4.0*r3*sin(a), 128.0, 128.0);
  // re = 0.4 * smoothStep(re, 0.1, 0.5);
  // mixed += re * (1.0 - mixed);

  // eye contours
  mixed=colsca(mixed,0.8+0.15*smoothStep(-b,0.0,0.2));
  mixed=colsca(mixed,0.8+0.2*smoothStep(-b,0.0,0.05));

  return mixed;
}

vec3 skin(float x, float y, float b) {
  float rx = 2.0*(x - 0.5)*demoloop_ScreenSize.x/demoloop_ScreenSize.y;
  float ry = 2.0*(y - 0.5);

  vec3 skinColor = vec3(0.75, 0.69, 0.6);

  // float cel = 0.95 + 0.05 * noise(64.0*x, 64.0*y, 256.0, 256.0);
  // skinColor = colsca(skinColor, cel);
  // skinColor.r += 0.03 * rx;
  // skinColor.g += 0.03 * ry;
  // skinColor = colsca(skinColor, y * 0.1 + 0.9);

  // float bri = noise(128.0 * x, 128.0 * y, 256.0, 256.0);
  // bri = 0.2 + 0.8 * smoothStep(bri, 0.0, 0.3);
  // skinColor = mix(bri*0.08*y, skinColor, vec3(1, 1, 1));

  // float san = 0.50*noise(16.0*x,16.0*y,256.0,256.0);
  //       san+= 0.25*noise(32.0*x,32.0*y,256.0,256.0);
  // skinColor.g*=1-0.1*san;

  // float osc = 0.500*noise(16.0*x,16.0*y,256.0,256.0);
  //       osc+= 0.250*noise(24.0*x,24.0*y,256.0,256.0);
  //       osc+= 0.125*noise(48.0*x,48.0*y,256.0,256.0);
  // skinColor=colsca(skinColor,0.9+0.1*osc);

  // skinColor.r+=0.08*x;
  // skinColor.g+=0.01;

  // float pecas = noise(32.0*x,32.0*y,256.0,256.0);
  // pecas=smoothStep(pecas,0.80,0.8001);
  // skinColor *= 1.0 - 0.16*pecas;

  float g = smoothStep(1.0 - b, 0.2, 0.7);
  g -= smoothStep(1.0 - b, 0.8, 1.0) * 3.0;
  skinColor += g * 0.1;

  // skinColor = mix(0.14, skinColor, vec3(1.0, 1.0, 1.0));
  // skinColor *= 1.23;
  // skinColor += 0.21;

  skinColor=colsca(skinColor,(1.0-(b*0.5)));

  return skinColor;
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  float r = 0.0, g = 0.0, b = 0.0;
  float x = tc.x;
  float y = tc.y;

  float rx = 2.0*(x - 0.5)*demoloop_ScreenSize.x/demoloop_ScreenSize.y;
  float ry = 2.0*(y - 0.5);
  // float h = 3.0*sqrt(x*x*x)*(1.0-x);
  float h = (2.0*(1.0 - pow(sin(cycle_ratio * DEMOLOOP_M_PI), 5.0)) + 1.0)*sqrt(x*x*x)*(1.0-x);
  float e = abs(ry) - h;
  float f = smoothStep( e, 0.0, 0.01 );

  float eyeX = x + cos(cycle_ratio * DEMOLOOP_M_PI * 2.0) * 0.1 - 0.1;
  // y += sin(cycle_ratio * DEMOLOOP_M_PI * 2) * 0.1;

  vec3 skinColor = skin(x, y, e);
  vec3 eyeColor = eye(eyeX, y, e);

  return vec4(mix(f, eyeColor, skinColor), 1.0);
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
