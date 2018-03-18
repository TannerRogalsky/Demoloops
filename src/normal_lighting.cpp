#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/3d_primitives.h"
#include "graphics/shader.h"
#include "graphics/mesh.h"
#include <array>
using namespace std;
using namespace demoloop;


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

glm::vec3 computeNormal(const Vertex &a, const Vertex &b, const Vertex &c) {
  glm::vec3 v1(a.x, a.y, a.z);
  glm::vec3 v2(b.x, b.y, b.z);
  glm::vec3 v3(c.x, c.y, c.z);
  return normalize(cross(v2 - v1, v3 - v1));
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
// const uint32_t numVertices = 4 * 3;
const glm::vec3 tetrahedronNormals[4 * 3] = {
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

#ifdef VERTEX
uniform mat4 v_inv;
attribute mat4 modelViews;
attribute vec3 v_normal;
attribute vec4 colors;

struct lightSource {
  vec4 position;
  vec4 diffuse;
  vec4 specular;
  float constantAttenuation, linearAttenuation, quadraticAttenuation;
  float spotCutoff, spotExponent;
  vec3 spotDirection;
};
lightSource light0 = lightSource(
  vec4(0.0,  -1.0,  0.0, 1.0),
  vec4(1.0,  1.0,  1.0, 1.0),
  vec4(1.0,  1.0,  1.0, 1.0),
  0.0, 1.0, 0.0,
  180.0, 0.0,
  vec3(0.0, 0.0, 0.0)
);
vec4 scene_ambient = vec4(0.6, 0.6, 0.6, 1.0);

struct material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};
material mymaterial = material(
  vec4(0.2, 0.2, 0.2, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  5.0
);

vec4 position(mat4 transform_proj, mat4 model, vec4 v_coord) {
  mat4 mvp = transform_proj * model;
  vec3 normalDirection = normalize(NormalMatrix * v_normal);
  vec3 viewDirection = normalize(vec3(v_inv * vec4(0.0, 0.0, 0.0, 1.0) - model * v_coord));
  vec3 lightDirection;
  float attenuation;

  if (light0.position.w == 0.0) { // directional light
    attenuation = 1.0; // no attenuation
    lightDirection = normalize(vec3(light0.position));
  } else { // point or spot light (or other kind of light)
    vec3 vertexToLightSource = vec3(light0.position - model * v_coord);
    float distance = length(vertexToLightSource);
    lightDirection = normalize(vertexToLightSource);
    attenuation = 1.0 / (light0.constantAttenuation
       + light0.linearAttenuation * distance
       + light0.quadraticAttenuation * distance * distance);

    if (light0.spotCutoff <= 90.0) { // spotlight
      float clampedCosine = max(0.0, dot(-lightDirection, normalize(light0.spotDirection)));
      if (clampedCosine < cos(radians(light0.spotCutoff))) { // outside of spotlight cone
        attenuation = 0.0;
      } else {
        attenuation = attenuation * pow(clampedCosine, light0.spotExponent);
      }
    }
  }

  vec3 ambientLighting = vec3(scene_ambient) * vec3(mymaterial.ambient);

  vec3 diffuseReflection = attenuation
    * vec3(light0.diffuse) * vec3(mymaterial.diffuse)
    * max(0.0, dot(normalDirection, lightDirection));

  vec3 specularReflection;
  if (dot(normalDirection, lightDirection) < 0.0) { // light source on the wrong side?
    specularReflection = vec3(0.0, 0.0, 0.0); // no specular reflection
  } else { // light source on the right side
    specularReflection = attenuation * vec3(light0.specular) * vec3(mymaterial.specular)
      * pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), mymaterial.shininess);
  }

  vColor = vec4(ambientLighting + diffuseReflection + specularReflection, 1.0);
  return mvp * v_coord;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords) {
  return Texel(texture, texture_coords) * vColor * color;
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

const uint32_t stacks = 30, slices = 30;
const uint32_t numVertices = (slices + 1) * (stacks + 1);
const uint32_t numIndices = slices * stacks * 6;

class Loop047 : public Demoloop {
public:
  Loop047() : Demoloop(CYCLE_LENGTH, 150, 150, 150), shader({shaderCode, shaderCode}), mesh(plane(5, 5, stacks, slices)) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    glGenBuffers(1, &modelViewsBuffer);
    glGenBuffers(1, &normalsBuffer);
    glGenBuffers(1, &colorsBuffer);

    // gl.bufferVertices((Vertex *)&tetrahedron, numVertices, GL_STATIC_DRAW);
    // glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    // glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    fill(begin(planeNormals), end(planeNormals), glm::vec3(0, 0, 0));
    uint32_t index = 0, sliceCount = slices + 1;
    for (uint32_t i = 0; i < stacks; ++i) {
      for (uint32_t j = 0; j < slices; ++j) {

        const uint32_t a = i * sliceCount + j;
        const uint32_t b = i * sliceCount + j + 1;
        const uint32_t c = ( i + 1 ) * sliceCount + j + 1;
        const uint32_t d = ( i + 1 ) * sliceCount + j;

        // faces one and two
        mesh.mIndices[index++] = a;
        mesh.mIndices[index++] = b;
        mesh.mIndices[index++] = d;
        glm::vec3 faceNormal1 = computeNormal(mesh.mVertices[a], mesh.mVertices[b], mesh.mVertices[d]);
        planeNormals[a] = glm::normalize(planeNormals[a] + faceNormal1);
        planeNormals[b] = glm::normalize(planeNormals[b] + faceNormal1);
        planeNormals[c] = glm::normalize(planeNormals[c] + faceNormal1);

        mesh.mIndices[index++] = b;
        mesh.mIndices[index++] = c;
        mesh.mIndices[index++] = d;
        glm::vec3 faceNormal2 = computeNormal(mesh.mVertices[b], mesh.mVertices[c], mesh.mVertices[d]);
        planeNormals[b] = glm::normalize(planeNormals[b] + faceNormal2);
        planeNormals[c] = glm::normalize(planeNormals[c] + faceNormal2);
        planeNormals[d] = glm::normalize(planeNormals[d] + faceNormal2);
      }
    }

    normalsLocation = shader.getAttribLocation("v_normal");
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const glm::vec3 eye = glm::rotate(glm::vec3(0, 1, 4), cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
    // const glm::vec3 eye = glm::vec3(0, 1, 4);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    glm::mat4 m;
    m = glm::rotate(m, (float)DEMOLOOP_M_PI / 2, {1, 0, 0});
    // m = glm::rotate(m, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0, 0, 1});

    shader.attach();

    // gl.bindTexture(texture);
    setColor(255, 255, 255);
    gl.prepareDraw(m);

    glm::mat4 v_inv = glm::inverse(t1.get());
    shader.sendMatrix("v_inv", 4, &v_inv[0][0], 1);
    // shader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    gl.bufferVertices(mesh.mVertices.data(), mesh.mVertices.size());
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &planeNormals[0].x, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    gl.bufferIndices(mesh.mIndices.data(), mesh.mIndices.size());

    gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD | (1u << normalsLocation));
    gl.drawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);



    for (uint32_t i = 0; i < 5; ++i) {
      const float i_cycle_ratio = fmod((float)i / 5 + cycle_ratio, 1);
      setColor(hsl2rgb(i_cycle_ratio, 1, 0.5));

      m = glm::mat4();
      m = glm::rotate(m, (float)DEMOLOOP_M_PI * i_cycle_ratio * 2, {0, 1, 0});
      m = glm::translate(m, {sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * 2, 0.5, 0});
      m = glm::scale(m, {4, 4, 4});
      m = glm::rotate(m, (float)DEMOLOOP_M_PI * cycle_ratio * 4, {0, 0, 1});
      gl.prepareDraw(m);

      gl.bufferVertices((Vertex *)tetrahedron, 4 * 3);
      glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
      glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
      glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

      glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
      glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &tetrahedronNormals[0].x, GL_DYNAMIC_DRAW);
      glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

      gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD | (1u << normalsLocation));
      gl.drawArrays(GL_TRIANGLES, 0, 4 * 3);
    }

    shader.detach();

    // for (uint32_t i = 0; i < numTris; ++i) {
    //   const float t = i;
    //   const float armIndex = i % arms;
    //   const float indexInArm = floor(t / arms) / trisPerArm;
    //   const float i_cycle_ratio = fmod(indexInArm + cycle_ratio, 1);
    //   // const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);

    //   glm::vec4 p(0, 0, 0, 0);
    //   matrices[i] = glm::mat4();
    //   glm::mat4& m = matrices[i];
    //   m = glm::rotate(m, -i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
    //   m = glm::rotate(m, armIndex / arms * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 0, 1));
    //   // m = glm::rotate(m, i_cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
    //   m = glm::translate(m, {i_cycle_ratio * 3, 0, 0});
    //   // m = glm::rotate(m, indexInArm * (float)DEMOLOOP_M_PI * 2, glm::vec3(1, 0, 1));

    //   colors[i] = hsl2rgbf(indexInArm, 1, 0.5);
    //   colors[i].a = 1 - pow(i_cycle_ratio, 5);
    // }

    // shader.attach();

    // uint32_t modelViewsLocation = shader.getAttribLocation("modelViews");
    // uint32_t enabledAttribs = bufferMatrixAttribute(modelViewsLocation, modelViewsBuffer, &matrices[0][0][0], numTris, GL_STREAM_DRAW);

    // gl.prepareDraw();

    // shader.sendFloat("time", 1, &t, 1);

    // // glm::mat4 v_inv = glm::inverse(t1.get());
    // // shader.sendMatrix("v_inv", 4, &v_inv[0][0], 1);

    // uint32_t colorsLocation = shader.getAttribLocation("colors");
    // glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
    // glBufferData(GL_ARRAY_BUFFER, numTris * sizeof(glm::vec4), &colors[0].x, GL_STREAM_DRAW);
    // glVertexAttribPointer(colorsLocation, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
    // glVertexAttribDivisor(colorsLocation, 1);

    // gl.useVertexAttribArrays(enabledAttribs | ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | (1u << normalsLocation) | (1u << colorsLocation));
    // gl.drawArraysInstanced(GL_TRIANGLES, 0, numVertices, numTris);
    // shader.detach();
  }

private:
  uint32_t normalsLocation;
  glm::mat4 matrices[numTris];
  glm::vec4 colors[numTris];
  glm::vec3 planeNormals[numIndices];
  Shader shader;
  Mesh mesh;
  GLuint modelViewsBuffer;
  GLuint normalsBuffer;
  GLuint colorsBuffer;
};

int main(int, char**){
  Loop047 test;
  test.Run();

  return 0;
}
