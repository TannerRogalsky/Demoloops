#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
// #include "graphics/2d_primitives.h"
#include "graphics/shader.h"
#include "graphics/canvas.h"
#include <vector>
using namespace std;
using namespace demoloop;


const uint32_t CYCLE_LENGTH = 10;
const uint32_t numVerts = 5;
const float lineWidthScale = 0.85;
// const float lineWidthScale = cosf(DEMOLOOP_M_PI / numVerts);


Vertex lines[numVerts * 2];
Triangle polygon[numVerts * 2];

const glm::vec3 twoDAxis(0, 0 , 1);
const RGB white(255, 255, 255);
const RGB pink(255, 132, 242); // 1.0, 0.52, 0.95
const RGB black(0, 0, 0);

const Vertex quad[6] = {
  {0, 0, 0, 0, 0}, {0, 1, 0, 0, 1}, {1, 0, 0, 1, 0},
  {1, 1, 0, 1, 1}, {0, 1, 0, 0, 1}, {1, 0, 0, 1, 0}
};

const static std::string noiseShaderCode = R"===(
#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec3 hash3( vec2 p ) {
  vec3 q = vec3( dot(p,vec2(127.1,311.7)),
           dot(p,vec2(269.5,183.3)),
           dot(p,vec2(419.2,371.9)) );
  return fract(sin(q)*43758.5453);
}

float iqnoise( in vec2 x, float u, float v ) {
  vec2 p = floor(x);
  vec2 f = fract(x);

  float k = 1.0+63.0*pow(1.0-v,4.0);

  float va = 0.0;
  float wt = 0.0;
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
  vec2 p = vec2(1.0, 1.0);
  float f = iqnoise(64.0*tc, p.x, p.y);
  // float f = fract(iqnoise(16.0*tc, p.x, p.y) * 16.0);

  return vec4(f, f, f, 1.0) * color;
}
#endif
)===";

const static std::string blurShaderCode = R"===(
#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
float d = 1.0*0.5 + 1.5;

float lookup(Image texture, vec2 p, float dx, float dy)
{
    vec2 uv = (p.xy + vec2(dx * d, dy * d)) / demoloop_ScreenSize.xy;
    vec4 c = texture2D(texture, uv.xy);

  // return as luma
    return 0.2126*c.r + 0.7152*c.g + 0.0722*c.b;
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  vec2 p = gl_FragCoord.xy;

  // simple sobel edge detection
  float gx = 0.0;
  gx += -1.0 * lookup(texture, p, -1.0, -1.0);
  gx += -2.0 * lookup(texture, p, -1.0,  0.0);
  gx += -1.0 * lookup(texture, p, -1.0,  1.0);
  gx +=  1.0 * lookup(texture, p,  1.0, -1.0);
  gx +=  2.0 * lookup(texture, p,  1.0,  0.0);
  gx +=  1.0 * lookup(texture, p,  1.0,  1.0);

  float gy = 0.0;
  gy += -1.0 * lookup(texture, p, -1.0, -1.0);
  gy += -2.0 * lookup(texture, p,  0.0, -1.0);
  gy += -1.0 * lookup(texture, p,  1.0, -1.0);
  gy +=  1.0 * lookup(texture, p, -1.0,  1.0);
  gy +=  2.0 * lookup(texture, p,  0.0,  1.0);
  gy +=  1.0 * lookup(texture, p,  1.0,  1.0);

  // hack: use g^2 to conceal noise in the video
  float g = gx*gx + gy*gy;
  // float g2 = g * (sin(iGlobalTime) / 2.0 + 0.5);
  float g2 = g;

  vec4 col = texture2D(texture, p / demoloop_ScreenSize.xy);
  col += vec4(vec3(1.0, 0.52, 0.95), g);
  // col = vec4(g, g, g, 1.0);
  // col = vec4(vec3(255 / 255.0, 132 / 255.0, 242 / 255.0) * g, 1.0);

  return col;
}
#endif
)===";


void setColor(const RGB &c) {
  glVertexAttrib4f(ATTRIB_CONSTANTCOLOR, c.r / 255.0, c.g / 255.0, c.b / 255.0, 1.0);
}

void pentagonalRose(float x, float y) {
  GL::TempTransform outsideT(gl);
  outsideT.get() = glm::translate(outsideT.get(), {x, y, 0});

  for (int i = 4; i >= 0; --i) {
    GL::TempTransform t1(gl);
    glm::mat4 &m = t1.get();

    const float scale = powf(cosf(DEMOLOOP_M_PI / numVerts), i);

    m = glm::rotate(m, (float)DEMOLOOP_M_PI / numVerts * i, twoDAxis);
    m = glm::scale(m, {scale, scale, 1});

    setColor(black);
    gl.triangles(polygon, numVerts * 2);
    setColor(pink);
    gl.lines(lines, numVerts * 2);

    {
      GL::TempTransform t2(gl);
      glm::mat4 &m = t2.get();

      m = glm::scale(m, {lineWidthScale, lineWidthScale, 1});
      gl.lines(lines, numVerts * 2);
    }
  }
}

void line(float x1, float y1, float x2, float y2) {
  const Vertex v[2] = {{x1, y1, 1}, {x2, y2, 1}};
  gl.bindTexture(gl.getDefaultTexture());
  gl.lines(v, 2);
}

void line(const glm::vec2 &a, const glm::vec2 &b) {
  line(a.x, a.y, b.x, b.y);
}

void lineSegment(const glm::vec2 &start, const vector<glm::vec2> &deltas) {
  glm::vec2 current(start);
  for (const glm::vec2 &delta : deltas) {
    line(current, current + delta);
    current += delta;
  }
  line(current, start);
}

class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(CYCLE_LENGTH, 500, 500, 0, 0, 0),
        noiseShader({noiseShaderCode, noiseShaderCode}),
        blurShader({blurShaderCode, blurShaderCode}),
        canvas(width, height),
        RADIUS(width / 3.0) {
    glDisable(GL_DEPTH_TEST);
    // gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
    // glLineWidth(2.0);

    const float interval = DEMOLOOP_M_PI * 2 / numVerts;
    float phi = -DEMOLOOP_M_PI / numVerts / 2;
    // const float interval = DEMOLOOP_M_PI * 2 / numVerts * 2;
    for (uint32_t i = 0; i < (numVerts * 2) - 1; i+=2, phi += interval) {
      float x1 = RADIUS * cosf(phi);
      float y1 = RADIUS * sinf(phi);
      float x2 = RADIUS * cosf(phi + interval);
      float y2 = RADIUS * sinf(phi + interval);

      lines[i].x = x1;
      lines[i].y = y1;
      lines[i].z = 1;

      lines[i + 1].x = x2;
      lines[i + 1].y = y2;
      lines[i + 1].z = 1;

      polygon[i] = {
        {x1, y1, 1},
        {x1 * lineWidthScale, y1 * lineWidthScale, 1},
        {x2, y2, 1}
      };

      polygon[i + 1] = {
        {x1 * lineWidthScale, y1 * lineWidthScale, 1},
        {x2 * lineWidthScale, y2 * lineWidthScale, 1},
        {x2, y2, 1}
      };
    }
  }

  void Update() {
    gl.bindTexture(gl.getDefaultTexture());
    setColor(white);

    // noiseShader.attach();
    // gl.triangles(quad, 6, glm::scale(glm::mat4(), {width, height, 1}));
    // noiseShader.detach();

    // pentagonalRose(width / 2.0, height / 2.0);

    setColor(pink);

    setCanvas(&canvas);
    const float interval = DEMOLOOP_M_PI * 2 / numVerts;
    float phi = 0;
    // float phi = cycle_ratio * DEMOLOOP_M_PI * 2;
    for (uint32_t i = 0; i < numVerts; ++i, phi += interval) {
      GL::TempTransform t2(gl);
      glm::mat4 &m = t2.get();
      m = glm::translate(m, {250, 250, 0});
      m = glm::rotate(m, phi, twoDAxis);
      m = glm::translate(m, {-250, -250, 0});
      m = glm::translate(m, {0, -265.0 / 2.0, 0});
      {
        const glm::vec2 start(250.54945,137.63736);
        const vector<glm::vec2> deltas = {
           {-115,82},
           {-33,101},
           {31,-23},
           {17,-54},
           {98,-71},
           {58,42},
           {50,0},
        };

        lineSegment(start, deltas);
      }

      {
        const glm::vec2 start(249.45055,221.15385);
        const vector<glm::vec2> deltas = {
          {-54.39561,0.54945},
          {-31.86813,23.07692},
          {87.36264,0.54945},
          {36.26374,26.92308},
          {32.41758,0},
        };
        lineSegment(start, deltas);
      }

      {
        const glm::vec2 start(239.56044,250.82418);
        const vector<glm::vec2> deltas = {
          {-30.21978,-0.54945},
          {-36.26374,26.92307},
          {-20.87912,65.93407},
          {20.87912,-14.28572},
          {12.63737,-38.46153},
        };
        lineSegment(start, deltas);
      }

      {
        const glm::vec2 start(250,276.64835);
        const vector<glm::vec2> deltas = {
          {-37.91209,0},
          {-18.13186,14.28571},
          {56.04395,0.54945},
          {80.76923,58.79121},
          {-6.04396,-20.87912},
        };
        lineSegment(start, deltas);
      }
    }
    setCanvas();

    setColor(white);
    blurShader.attach();
    canvas.draw();
    blurShader.detach();
  }

private:
  Shader noiseShader;
  Shader blurShader;
  Canvas canvas;
  const float RADIUS;
};

int main(int, char**){
  Loop050 test;
  test.Run();

  return 0;
}
