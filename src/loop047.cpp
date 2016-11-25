// #include <iostream>
#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/3d_primitives.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;
const uint32_t arms = 20;
const uint32_t trisPerArm = 20;
const uint32_t numTris = arms * trisPerArm;

glm::vec3 computeNormal(const Triangle &t) {
  glm::vec3 v1(t.a.x, t.a.y, t.a.z);
  glm::vec3 v2(t.b.x, t.b.y, t.b.z);
  glm::vec3 v3(t.c.x, t.c.y, t.c.z);
  return normalize(cross(v2 - v1, v3 - v1));
}

glm::vec3 computeNormal(const Triangle &t1, const Triangle &t2, const Triangle &t3) {
  return normalize(computeNormal(t1) + computeNormal(t2) + computeNormal(t3));
}

const float RADIUS = 0.1;
const float interval = DEMOLOOP_M_PI * 2 / 3;
const Triangle triangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 0},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 0},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 0}
};
const Vertex peak = {0, 0, sqrtf(2) * RADIUS};
const Triangle tetrahedron[4] = {
  {triangle.a, triangle.c, triangle.b}, // bottom
  {triangle.a, triangle.b, peak}, // a - b
  {triangle.b, triangle.c, peak}, // b - c
  {triangle.c, triangle.a, peak}, // c - a
};
const uint32_t numVertices = 4 * 3;
const glm::vec3 normals[numVertices] = {
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]),
  computeNormal(tetrahedron[0], tetrahedron[2], tetrahedron[3]),
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]),

  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]),
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]),
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]),

  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]),
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]),
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]),

  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]),
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]),
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]),
};

const static std::string shaderCode = R"===(
varying vec4 vpos;
varying vec3 vNorm;
varying vec4 vColor;

vec3 lightDir(0, 0, 1);

#ifdef VERTEX
attribute mat4 modelViews;
attribute vec3 normals;
attribute vec4 colors;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  // vpos = normalize(vertpos);
  vColor = colors;
  vNorm = NormalMatrix * normals;
  return transform_proj * modelViews * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords) {
  float cosTheta = clamp(dot(vNorm, lightDir), 0, 1);
  return Texel(texture, texture_coords) * vColor * vec4(vNorm, 1.0) * color;
  // return vColor;
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

class Loop047 : public Demoloop {
public:
  Loop047() : Demoloop(150, 150, 150), shader({shaderCode, shaderCode}) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    glGenBuffers(1, &modelViewsBuffer);
    glGenBuffers(1, &normalsBuffer);
    glGenBuffers(1, &colorsBuffer);

    gl.bufferVertices((Vertex *)&tetrahedron, numVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    normalsLocation = shader.getAttribLocation("normals");
    glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &normals[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    const glm::vec3 eye = glm::rotate(glm::vec3(4, 1, 4), cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(-0.3, 1, 0));
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    for (uint32_t i = 0; i < numTris; ++i) {
      const float t = i;
      const float armIndex = i % arms;
      const float indexInArm = floor(t / arms) / trisPerArm;
      const float i_cycle_ratio = fmod(indexInArm + cycle_ratio, 1);
      // const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);

      glm::vec4 p(0, 0, 0, 0);
      matrices[i] = glm::mat4();
      glm::mat4& m = matrices[i];
      m = glm::rotate(m, -i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
      m = glm::rotate(m, armIndex / arms * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 0, 1));
      // m = glm::rotate(m, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
      m = glm::translate(m, {i_cycle_ratio * 3, 0, 0});
      // m = glm::rotate(m, indexInArm * (float)DEMOLOOP_M_PI * 2, glm::vec3(1, 0, 1));

      colors[i] = hsl2rgbf(indexInArm, 1, 0.5);
      colors[i].a = 1 - pow(i_cycle_ratio, 5);
    }

    shader.attach();

    uint32_t modelViewsLocation = shader.getAttribLocation("modelViews");
    uint32_t enabledAttribs = bufferMatrixAttribute(modelViewsLocation, modelViewsBuffer, &matrices[0][0][0], numTris, GL_STREAM_DRAW);

    gl.prepareDraw();

    uint32_t colorsLocation = shader.getAttribLocation("colors");
    glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    glBufferData(GL_ARRAY_BUFFER, numTris * sizeof(glm::vec4), &colors[0].x, GL_STREAM_DRAW);
    glVertexAttribPointer(colorsLocation, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
    glVertexAttribDivisor(colorsLocation, 1);

    gl.useVertexAttribArrays(enabledAttribs | ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | (1u << normalsLocation) | (1u << colorsLocation));
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
  Loop047 test;
  test.Run();

  return 0;
}
