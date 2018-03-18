
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
highp vec3 hash3( vec2 p )
{
  highp vec3 q = vec3( dot(p,vec2(127.1,311.7)),
         dot(p,vec2(269.5,183.3)),
         dot(p,vec2(419.2,371.9)) );
  return fract(sin(q)*43758.5453);
}

highp float iqnoise( in vec2 x, float u, float v ) {
  vec2 p = floor(x);
  vec2 f = fract(x);

  highp float k = 1.0+63.0*pow(1.0-v,4.0);

  highp float va = 0.0;
  highp float wt = 0.0;
  for( int j=-2; j<=2; j++ )
  for( int i=-2; i<=2; i++ ) {
    vec2 g = vec2( float(i),float(j) );
    vec3 o = hash3( p + g )*vec3(u,u,1.0);
    vec2 r = g - f + o.xy;
    float d = dot(r,r);
    float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );
    va += o.z*ww;
    wt += ww;
  }

  return va/wt;
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  float x = tc.x;
  float y = tc.y;

  //u=0, v=0: Cell Noise
  //u=0, v=1: Noise
  //u=1, v=0: Voronoi
  //u=1, v=1: Voronoise

  vec2 cellNoise = vec2(0.0, 0.0);
  vec2 noise =     vec2(0.0, 1.0);
  vec2 voronoi =   vec2(1.0, 0.0);
  vec2 voronoise = vec2(1.0, 1.0);

  // vec2 p = mix(noise, voronoise, pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2.0));
  vec2 p = mix(cellNoise, voronoi, pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2.0));

  p = p*p*(3.0-2.0*p);
  p = p*p*(3.0-2.0*p);
  p = p*p*(3.0-2.0*p);

  float f = iqnoise( 24.0*tc, p.x, p.y );

  return vec4( f, f, f, 1.0 );
}
#endif
)===";
class Voronoi : public Demoloop {
public:
  Voronoi() : Demoloop(CYCLE_LENGTH, 150, 150, 150), shader({shaderCode, shaderCode}) {
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
  Voronoi test;
  test.Run();

  return 0;
}
