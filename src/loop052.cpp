#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
using namespace std;
using namespace demoloop;

const uint32_t CYCLE_LENGTH = 10;

const uint32_t slices = 20;
const uint32_t stacks = 20;
const uint32_t depth = 8;
const uint32_t numTris = slices * stacks * depth;
// const uint32_t numTris = 1;

glm::vec3 computeNormal(const Triangle &t) {
  glm::vec3 v1(t.a.x, t.a.y, t.a.z);
  glm::vec3 v2(t.b.x, t.b.y, t.b.z);
  glm::vec3 v3(t.c.x, t.c.y, t.c.z);
  return normalize(cross(v2 - v1, v3 - v1));
}

glm::vec3 computeNormal(const Triangle &t1, const Triangle &t2, const Triangle &t3) {
  return normalize(computeNormal(t1) + computeNormal(t2) + computeNormal(t3));
}

const float RADIUS = 1.0;
const float interval = DEMOLOOP_M_PI * 2 / 3;
const Triangle triangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 0, 0, 0, 255, 255, 255, 255},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 0, 0, 1, 255, 255, 255, 255},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 0, 1, 0, 255, 255, 255, 255}
};
const Vertex peak = {0, 0, sqrtf(2) * RADIUS, 1, 1, 255, 255, 255, 255};
const Triangle tetrahedron[4] = {
  {triangle.a, triangle.c, triangle.b}, // bottom
  {triangle.a, triangle.b, peak},       // A
  {triangle.b, triangle.c, peak},       // B
  {triangle.c, triangle.a, peak},       // C
};
const uint32_t numVertices = 4 * 3;
const glm::vec3 normals[numVertices] = {
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]), // bottom1
  computeNormal(tetrahedron[0], tetrahedron[2], tetrahedron[3]), // bottom2
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]), // bottom3

  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]), // A1
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]), // A2
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]), // A3

  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]), // B1
  computeNormal(tetrahedron[0], tetrahedron[2], tetrahedron[3]), // B2
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]), // B3

  computeNormal(tetrahedron[0], tetrahedron[2], tetrahedron[3]), // C1
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]), // C2
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]), // C3
};

const static std::string shaderCode = R"===(
varying vec4 vpos;
varying vec3 vNorm;
varying vec4 vColor;

uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
attribute mat4 modelViews;
attribute vec3 normals;
attribute vec4 colors;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vColor = colors;
  vNorm = normals;
  return transform_proj * modelViews * vertpos;
}
#endif

#ifdef PIXEL
vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  vec2 bl = mix(vec2(1.0), step(vec2(0.1), tc), 0.5); ;
  bl = mix(bl, step(vec2(0.2), tc), 0.5);
  bl = mix(bl, step(vec2(0.3), tc), 0.5);
  bl = mix(bl, step(vec2(0.4), tc), 0.5);
  // bl = mix(bl, step(vec2(0.5), tc), 0.5);

  vec2 tr = mix(vec2(1.0), step(vec2(0.1), 1.0 - tc), 0.5); ;
  tr = mix(tr, step(vec2(0.2), 1.0 - tc), 0.5);
  tr = mix(tr, step(vec2(0.3), 1.0 - tc), 0.5);
  tr = mix(tr, step(vec2(0.4), 1.0 - tc), 0.5);
  // tr = mix(tr, step(vec2(0.5), 1.0 - tc), 0.5);

  float pct = bl.x * bl.y * tr.x * tr.y;
  float t = mod(cycle_ratio + pct + vColor.r, 1.0);

  return vec4(hsv2rgb(vec3(t, 1.0, 0.8)), vColor.a);
}
#endif
)===";

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

const float farPlane = 80.0;

class Loop052 : public Demoloop {
public:
  Loop052() : Demoloop(CYCLE_LENGTH, 0, 0, 0), shader({shaderCode, shaderCode}) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, farPlane);
    gl.getProjection() = perspective;

    glGenBuffers(1, &modelViewsBuffer);
    glGenBuffers(1, &normalsBuffer);
    glGenBuffers(1, &colorsBuffer);

    gl.bufferVertices((Vertex *)&tetrahedron, numVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    normalsLocation = shader.getAttribLocation("normals");
    glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &normals[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    float eyeRot = 0;
    // eyeRot += cycle_ratio * DEMOLOOP_M_PI * 2;
    // eyeRot += DEMOLOOP_M_PI / 2;

    const float eyePosScale = 3.2;
    // const float eyePosScale = 0;
    const glm::vec3 eyePos = {
      cosf(cycle_ratio * DEMOLOOP_M_PI * 2) * eyePosScale,
      sinf(cycle_ratio * DEMOLOOP_M_PI * 2) * eyePosScale,
      0,
    };

    // const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 0), eyeRot, glm::vec3(-0.3, 1, 0));
    const glm::vec3 eye = glm::rotate(eyePos, eyeRot, glm::vec3(-0.3, 1, 0));
    // const glm::vec3 eye = glm::rotate(glm::vec3(0, 4, 40), eyeRot, glm::vec3(-0.3, 1, 0));
    // const glm::vec3 eye = glm::rotate(eyePos, eyeRot, glm::vec3(-0.3, 1, 0));
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, farPlane);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    const glm::vec3 twoDAxis = {0, 0, 1};
    for (uint32_t y = 0; y < stacks; ++y) {
      for (uint32_t x = 0; x < slices; ++x) {
        for (uint32_t z = 0; z < depth; ++z) {
          uint32_t i = x + slices * (y + stacks * z);
          float t = i;
          const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);
          const float pseudoRandom = sinf(t / numTris * 9999999);

          glm::mat4& m = matrices[i] = glm::mat4();
          // m = glm::rotate(m, (float)DEMOLOOP_M_PI * 2 / arms * armIndex, twoDAxis);

          float px = 0, py = 0, pz = 0;

          // pz += (0.9 - (float)z / (float)depth) * farPlane;
          pz += (1.0 - fmod((float)z / depth + cycle_ratio, 1.0)) * farPlane;
          px += ((float)(x - slices / 2.0 + 0.5) / slices) * 50;
          py += ((float)(y - stacks / 2.0 + 0.5) / stacks) * 50;

          m = glm::translate(m, {px, py, pz});

          m = glm::rotate(m, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2 + pseudoRandom, {0, 1, 1});

          // colors[i] = hsl2rgbf(t / numTris, 1, 0.5);
          colors[i].r = pow(pseudoRandom, 2.0);
          colors[i].g = 0;
          colors[i].b = 0;
          colors[i].a = 1 - pow(pz / farPlane, 5);
          // colors[i].a = 1 - pow(1 - i_cycle_ratio, 5);
        }
      }
    }

    shader.attach();
    shader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    uint32_t modelViewsLocation = shader.getAttribLocation("modelViews");
    uint32_t enabledAttribs = bufferMatrixAttribute(modelViewsLocation, modelViewsBuffer, &matrices[0][0][0], numTris, GL_STREAM_DRAW);

    gl.prepareDraw();

    uint32_t colorsLocation = shader.getAttribLocation("colors");
    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    glBufferData(GL_ARRAY_BUFFER, numTris * sizeof(glm::vec4), &colors[0].x, GL_STREAM_DRAW);
    glVertexAttribPointer(colorsLocation, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
    glVertexAttribDivisor(colorsLocation, 1);

    enabledAttribs |= ATTRIBFLAG_POS |
                      ATTRIBFLAG_COLOR |
                      ATTRIBFLAG_TEXCOORD |
                      (1u << normalsLocation) |
                      (1u << colorsLocation);
    gl.useVertexAttribArrays(enabledAttribs);
    gl.drawArraysInstanced(GL_TRIANGLES, 0, numVertices, numTris);
    shader.detach();
  }

private:
  uint32_t normalsLocation;
  glm::mat4 matrices[numTris];
  glm::vec4 colors[numTris];
  Shader shader;
  GLuint modelViewsBuffer;
  GLuint normalsBuffer;
  GLuint colorsBuffer;
};

int main(int, char**){
  Loop052 test;
  test.Run();

  return 0;
}
