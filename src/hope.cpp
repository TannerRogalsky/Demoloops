#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
#include "audio/source.h"

#include <functional>
#include <vector>

using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;
const uint32_t arms = 5;
const uint32_t trisPerArm = 40;
const uint32_t numTris = arms * trisPerArm;

const float RADIUS = 10;
const float interval = DEMOLOOP_M_PI * 2 / 3;
const Triangle triangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 0},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 0},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 0}
};
const uint32_t numVertices = 3;

const static std::string blurShaderCode = R"===(
#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
extern vec2 direction;

vec4 blur(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  color += texture2D(image, uv) * 0.2270270270;
  color += texture2D(image, uv + (off1 / resolution)) * 0.3162162162;
  color += texture2D(image, uv - (off1 / resolution)) * 0.3162162162;
  color += texture2D(image, uv + (off2 / resolution)) * 0.0702702703;
  color += texture2D(image, uv - (off2 / resolution)) * 0.0702702703;
  return color;
}

vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords) {
  vec4 texturecolor = blur(texture, texture_coords, demoloop_ScreenSize.xy, direction);
  return texturecolor * color;
}
#endif

)===";

const static std::string shaderCode = R"===(
varying vec4 vpos;
varying vec4 vColor;

#ifdef VERTEX
attribute mat4 modelViews;
attribute vec4 colors;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vColor = colors;
  return transform_proj * modelViews * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords) {
  return Texel(texture, texture_coords) * vColor;
}
#endif
)===";

float cubicEaseIn (float t,float b , float c, float d) {
  t /= d;
  return c*t*t*t + b;
}
float cubicEaseOut(float t,float b , float c, float d) {
  t = t / d - 1;
  return c*(t*t*t + 1) + b;
}

float cubicEaseInOut(float t,float b , float c, float d) {
  t/=d/2;
  if (t < 1) return c/2*t*t*t + b;
  t-=2;
  return c/2*(t*t*t + 2) + b;
}

uint32_t bufferMatrixAttribute(const GLint location, const GLuint buffer, const float *data, const uint32_t num, GLenum usage) {
  uint32_t enabledAttribs = 0;
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, num * sizeof(float) * 4 * 4, data, usage);

  for (int i = 0; i < 4; ++i) {
    enabledAttribs |= 1u << (uint32_t)(location + i);
    glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*)(sizeof(float) * (4 * i)));
    glVertexAttribDivisor(location + i, 1);
  }

  return enabledAttribs;
}

glm::vec3 loop1(float cycle_ratio, uint32_t i) {
  const float t = i;
  const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);
  float phi = i_cycle_ratio * DEMOLOOP_M_PI * 2;

  float x = 16 * powf(sinf(phi), 3);
  x *= 10;
  float y = 13 * cosf(phi) - 5 * cosf(2 * phi) - 2 * cosf(3 * phi) - cosf(4 * phi);
  y *= 10;

  return glm::vec3(x, y, 0);
}

glm::vec3 loop2(float cycle_ratio, uint32_t i) {
  const float t = i;
  const float armIndex = i % arms;
  const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);
  float phi = DEMOLOOP_M_PI * 2 / arms * armIndex;

  float x = cosf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * 160;
  x *= sinf(cycle_ratio * DEMOLOOP_M_PI * 2);
  x += powf(sinf(i_cycle_ratio * DEMOLOOP_M_PI), 2) * 160 / 2;
  float y = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * 160;
  y *= cosf(cycle_ratio * DEMOLOOP_M_PI * 2);

  {
    float c = cosf(phi);
    float s = sinf(phi);
    float tx = x;
    float ty = y;
    x = c*tx - s*ty;
    y = s*tx + c*ty;
  }

  return glm::vec3(x, y, 0);
}

glm::vec3 loop3(float cycle_ratio, uint32_t i) {
  const float t = i;
  const float armIndex = i % arms;
  const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);
  float phi = DEMOLOOP_M_PI * 2 / arms * armIndex;

  float size = 160;

  float x = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * size;
  x = x + powf(sinf(i_cycle_ratio * DEMOLOOP_M_PI), 2) * size / 2;
  float y = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * size * i_cycle_ratio;
  y = y + powf(sinf(i_cycle_ratio * DEMOLOOP_M_PI), 2) * size / 2;

  {
    float c = cosf(phi);
    float s = sinf(phi);
    float tx = x;
    float ty = y;
    x = c*tx - s*ty;
    y = s*tx + c*ty;
  }

  return glm::vec3(x, y, 0);
}

glm::vec3 loop4(float cycle_ratio, uint32_t i) {
  const float t = i;
  const float armIndex = i % arms;
  float indexInArm = floor(t / arms) / trisPerArm;
  float i_cycle_ratio = fmod(indexInArm + cycle_ratio, 1);
  float phi = DEMOLOOP_M_PI * 2 / arms * armIndex;

  float size = 160;

  float x = cosf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * size * sinf(cycle_ratio * DEMOLOOP_M_PI * 2);
  float y = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * size;

  {
    float c = cosf(phi);
    float s = sinf(phi);
    float tx = x;
    float ty = y;
    x = c*tx - s*ty;
    y = s*tx + c*ty;
  }

  return glm::vec3(x, y, 0);
}

vector<function<glm::vec3(float, uint32_t)>> loops = {
  loop1, loop2, loop3, loop4
};

class Hope : public Demoloop {
public:
  Hope() : Demoloop(150, 150, 150),
          shader({shaderCode, shaderCode}),
          blurShader({blurShaderCode, blurShaderCode}),
          source("hope/hopeneverstops.ogg") {
    glDisable(GL_DEPTH_TEST);

    glm::mat4 &m = gl.getTransform();
    m = glm::translate(m, {width / 2, height / 2, 0});

    bg_texture = loadTexture("hope/bg.png");

    glGenBuffers(1, &modelViewsBuffer);
    glGenBuffers(1, &colorsBuffer);

    source.play();
  }

  void Update(float dt) {
    t += dt;

    source.update();

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    float bpm = 96.0 / (60.0 / CYCLE_LENGTH);
    float beat_ratio = cycle_ratio * DEMOLOOP_M_PI * bpm;
    float beat = powf(cosf(beat_ratio), 4 * 8);

    blurShader.attach();
    float blurDirection[2] = {beat * 2, 0};
    blurShader.sendFloat("direction", 2, blurDirection, 1);
    renderTexture(bg_texture, -width / 2, -height / 2, width, height);

    uint32_t numLoops = loops.size();
    uint32_t activeLoop = fmod(floor(t / CYCLE_LENGTH), numLoops);

    const glm::vec3 twoDAxis = {0, 0 , 1};
    for (uint32_t i = 0; i < numTris; ++i) {

      glm::vec3 p1 = loops[activeLoop](cycle_ratio, i);
      glm::vec3 p2 = loops[(activeLoop + 1) % numLoops](cycle_ratio, i);

      // const float interp = cubicEaseInOut(powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2), 0, 1, 1);
      const float interp = cubicEaseInOut(cycle_ratio, 0, 1, 1);
      auto p = glm::mix(p1, p2, interp);

      glm::mat4& m = matrices[i] = glm::mat4();
      m = glm::rotate(m, (float)DEMOLOOP_M_PI * 2 * cycle_ratio + (float)DEMOLOOP_M_PI, twoDAxis);
      const float scale = 1 + beat / 8;
      m = glm::scale(m, {scale, scale, 1});
      m = glm::translate(m, p);

      float d = glm::length(p);
      colors[i] = hsl2rgbf(fmod(cycle_ratio + d / (width / 4) * 0.65, 1), 1, 0.5);
    }

    shader.attach();

    uint32_t modelViewsLocation = shader.getAttribLocation("modelViews");
    uint32_t enabledAttribs = bufferMatrixAttribute(modelViewsLocation, modelViewsBuffer, &matrices[0][0][0], numTris, GL_STREAM_DRAW);

    gl.prepareDraw();

    gl.bufferVertices((Vertex *)&triangle, numVertices, GL_STREAM_DRAW);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    uint32_t colorsLocation = shader.getAttribLocation("colors");
    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    glBufferData(GL_ARRAY_BUFFER, numTris * sizeof(glm::vec4), &colors[0].x, GL_STREAM_DRAW);
    glVertexAttribPointer(colorsLocation, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
    glVertexAttribDivisor(colorsLocation, 1);

    gl.bindTexture(gl.getDefaultTexture());
    gl.useVertexAttribArrays(enabledAttribs | ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | (1u << colorsLocation));
    gl.drawArraysInstanced(GL_TRIANGLES, 0, numVertices, numTris);
    shader.detach();
  }

private:
  GLuint bg_texture;
  glm::mat4 matrices[numTris];
  glm::vec4 colors[numTris];
  Shader shader;
  Shader blurShader;
  GLuint modelViewsBuffer;
  GLuint colorsBuffer;
  Source source;
};

int main(int, char**){
  Hope test;
  test.Run();

  return 0;
}
