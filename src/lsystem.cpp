
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
uniform mediump float iGlobalTime;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
#define kDepth 7
#define kBranches 3
#define kMaxDepth 2187 // branches ^ depth

//--------------------------------------------------------------------------

mat3 matRotate(float angle) {
  float c = cos(angle);
  float s = sin(angle);
  return mat3( c, s, 0, -s, c, 0, 0, 0, 1);
}

mat3 matTranslate( float x, float y ) {
  return mat3( 1, 0, 0, 0, 1, 0, -x, -y, 1 );
}

float sdBranch( vec2 p, float w1, float w2, float l ) {
  float h = clamp( p.y/l, 0.0, 1.0 );
  float d = length( p - vec2(0.0,l*h) );
  return d - mix( w1, w2, h );
}

//--------------------------------------------------------------------------

float map( vec2 pos ) {
  const float len = 3.2;
  const float wid = 0.3;
  const float lenf = 0.6;
  const float widf = 0.4;

  float d = sdBranch( pos, wid, wid*widf, len );

  float t = cycle_ratio * DEMOLOOP_M_PI * 2.0;

  int c = 0;
  for( int count=0; count < kMaxDepth; count++ ) {
    int off = kMaxDepth;
    vec2 pt_n = pos;

    float l = len;
    float w = wid;

    for( int i=1; i<=kDepth; i++ ) {
      l *= lenf;
      w *= widf;

      off /= kBranches;
      int dec = c / off;
      int path = dec - kBranches*(dec/kBranches); //  dec % kBranches

      mat3 mx;
      if( path == 0 ) {
        mx = matRotate(0.75 + 0.25*sin(t-1.0)) * matTranslate( 0.0,0.4*l/lenf);
      } else if( path == 1 ) {
        mx = matRotate(-0.6 + 0.21*sin(t)) * matTranslate(0.0,0.6*l/lenf);
      } else {
        mx = matRotate(0.23*sin(t+1.0)) * matTranslate(0.0,1.0*l/lenf);
      }
      pt_n = (mx * vec3(pt_n,1)).xy;

      // bounding sphere test
      float y = length( pt_n - vec2(0.0, l) );
      if( y-1.4*l > 0.0 ) { c += off-1; break; }

      d = min( d, sdBranch( pt_n, w, w*widf, l ) );
    }

    c++;
    if( c > kMaxDepth ) break;
  }

  return d;
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  // coordinate system
  vec2 uv = (tc - 0.5) * vec2(1, -1) * 2.0;
  float px = 2.0/demoloop_ScreenSize.y;

  // frame in screen
  uv = uv*4.0 + vec2(0.0,3.5);
  px = px*4.0;

  // compute
  float d = map( uv );

  // shape
  vec3 col = vec3( smoothstep( 0.0, 2.0*px, d ) );

  return vec4( col, 1.0 );
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
    // shader.sendFloat("iGlobalTime", 1, &t, 1);

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
