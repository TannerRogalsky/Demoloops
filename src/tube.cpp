#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/shader.h"
#include "helpers.h"
#include <array>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 30;

template <
  typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
> constexpr T mix(T const& a, T const& b, const float& ratio) {
  return a * (1.0f - ratio) + b * ratio;
}

glm::vec3 computeNormal(const Vertex &a, const Vertex &b, const Vertex &c) {
  glm::vec3 v1(a.x, a.y, a.z);
  glm::vec3 v2(b.x, b.y, b.z);
  glm::vec3 v3(c.x, c.y, c.z);
  return normalize(cross(v2 - v1, v3 - v1));
}

Vertex mix(const Vertex& a, const Vertex& b, const float& ratio) {
  const float x = mix<float>(a.x, b.x, ratio);
  const float y = mix<float>(a.y, b.y, ratio);
  const float z = mix<float>(a.z, b.z, ratio);

  const float s = mix<float>(a.s, b.s, ratio);
  const float t = mix<float>(a.t, b.t, ratio);

  const uint8_t red =   mix<uint8_t>(a.r, b.r, ratio);
  const uint8_t green = mix<uint8_t>(a.g, b.g, ratio);
  const uint8_t blue =  mix<uint8_t>(a.b, b.b, ratio);
  const uint8_t alpha = mix<uint8_t>(a.a, b.a, ratio);

  return {x, y, z, s, t, red, green, blue, alpha};
}

const static std::string shaderCode = R"===(
#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 m, vec4 v_coord) {
  mat4 mvp = transform_proj * m;
  return mvp * v_coord;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 st, vec2 screen_coords) {
  return Texel(texture, st) * color;
}
#endif
)===";

glm::vec3 granny_path(float t) {
  t = 2 * t;
  float x = -0.22 * cos(t) - 1.28 * sin(t) - 0.44 * cos(3 * t) - 0.78 * sin(3 * t);
  float y = -0.1 * cos(2 * t) - 0.27 * sin(2 * t) + 0.38 * cos(4 * t) + 0.46 * sin(4 * t);
  float z = 0.7 * cos(3 * t) - 0.4 * sin(3 * t);
  return {x, y, z};
}

glm::vec3 perp(const glm::vec3 &u) {
  // Randomly pick a reasonable perpendicular vector
  glm::vec3 u_prime = glm::cross(u, {1, 0, 0});
  if (glm::length(u_prime) < 0.01) { // v1 X x2 == 0 if they are parallel
    u_prime = glm::cross(u, {0, 1, 0});
  }
  return glm::normalize(u_prime);
}

glm::mat3 new_rotate_triple_axis(const glm::vec3 &x, const glm::vec3 &y, const glm::vec3 &z) {
  glm::mat3 m;

  m[0][0] = x.x;
  m[1][0] = y.x;
  m[2][0] = z.x;

  m[0][1] = x.y;
  m[1][1] = y.y;
  m[2][1] = z.y;

  m[0][2] = x.z;
  m[1][2] = y.z;
  m[2][2] = z.z;

  return m;
}

Vertex tube(const float s, const float t, const std::function<glm::vec3(float)> func, const float radius) {
  const float u = s * DEMOLOOP_M_PI, v = t * DEMOLOOP_M_PI;

  // Compute three basis vectors
  glm::vec3 p1 = func(u);
  glm::vec3 p2 = func(u + 0.01);
  glm::vec3 A = glm::normalize(p2 - p1);
  glm::vec3 B = perp(A);
  glm::vec3 C = glm::normalize(glm::cross(A, B));

  // Rotate the Z-plane circle appropriately
  glm::mat3 m = new_rotate_triple_axis(B, C, A);
  glm::vec3 spoke_vector = m * glm::vec3(cos(2*v), sin(2*v), 0);

  // Add the spoke vector to the center to obtain the rim position:
  glm::vec3 center = p1 + radius * spoke_vector;
  return {
    center.x, center.y, center.z,
    s * 20, t
  };
}

const uint32_t stacks = 10, slices = 30;
const uint32_t numVertices = (slices + 1) * (stacks + 1);
const uint32_t numIndices = slices * stacks * 6;

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(CYCLE_LENGTH, 150, 150, 150), shader({shaderCode, shaderCode}), sphereMesh(sphere(0.05)) {
    // glEnable(GL_CULL_FACE);
    texture = loadTexture("uv_texture.jpg");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  }

  ~Loop055() {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();
    const float mod_ratio = powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2);

    // const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 10), static_cast<float>(cycle_ratio * DEMOLOOP_M_PI * 2), glm::vec3(0, 1, 0));
    // const glm::vec3 eye = granny_path(cycle_ratio * DEMOLOOP_M_PI);
    const glm::vec3 eye = glm::vec3(0, 0, 10);
    const glm::vec3 target = {0, 0, 0};
    const glm::vec3 up = {0, 1, 0};
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    GL::TempProjection p1(gl);
    p1.get() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    const uint32_t sliceCount = slices + 1;

    uint32_t index = 0;
    for (uint32_t i = 0; i <= stacks; ++i) {
      const float v = static_cast<float>(i) / stacks;

      for (uint32_t j = 0; j <= slices; ++j) {
        const float u = static_cast<float>(j) / slices;

        Vertex vert = tube(u, v, granny_path, 0.1);
        vertices[index] = vert;
        normals[index] = glm::vec3(0, 0, 0);

        // sphereMesh.draw(glm::translate(glm::mat4(), granny_path(v * DEMOLOOP_M_PI)));
        // sphereMesh.draw();

        index++;
      }
    }

    index = 0;
    for (uint32_t i = 0; i < stacks; ++i) {
      for (uint32_t j = 0; j < slices; ++j) {

        const uint32_t a = i * sliceCount + j;
        const uint32_t b = i * sliceCount + j + 1;
        const uint32_t c = ( i + 1 ) * sliceCount + j + 1;
        const uint32_t d = ( i + 1 ) * sliceCount + j;

        // faces one and two
        indices[index++] = a;
        indices[index++] = b;
        indices[index++] = d;
        glm::vec3 faceNormal1 = computeNormal(vertices[a], vertices[b], vertices[d]);
        normals[a] = glm::normalize(normals[a] + faceNormal1);
        normals[b] = glm::normalize(normals[b] + faceNormal1);
        normals[c] = glm::normalize(normals[c] + faceNormal1);

        indices[index++] = b;
        indices[index++] = c;
        indices[index++] = d;
        glm::vec3 faceNormal2 = computeNormal(vertices[b], vertices[c], vertices[d]);
        normals[b] = glm::normalize(normals[b] + faceNormal2);
        normals[c] = glm::normalize(normals[c] + faceNormal2);
        normals[d] = glm::normalize(normals[d] + faceNormal2);
      }
    }

    shader.attach();

    gl.bufferVertices(&vertices[0], numVertices);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    // uint32_t normalsLocation = shader.getAttribLocation("v_normal");
    // glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
    // glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &normals[0].x, GL_DYNAMIC_DRAW);
    // glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    gl.bufferIndices(&indices[0], numIndices);

    // gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD | (1u << normalsLocation));
    gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD);

    {
      glm::mat4 m;
      // m = glm::translate(m, {0, 0, 0});
      m = glm::scale(m, {3, 3, 3});
      // m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));

      gl.prepareDraw(m);
      gl.bindTexture(texture);
      gl.drawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    }

    {
      const float u = cycle_ratio * DEMOLOOP_M_PI;
      // Compute three basis vectors
      glm::vec3 p1 = granny_path(u);
      glm::vec3 p2 = granny_path(u + 0.01);
      glm::vec3 A = glm::normalize(p2 - p1);
      glm::vec3 B = perp(A);
      glm::vec3 C = glm::normalize(glm::cross(A, B));

      // Rotate the Z-plane circle appropriately
      glm::mat3 m = new_rotate_triple_axis(B, C, A);
      glm::vec3 spoke_vector = m * glm::vec3(1, 0, 0);


      glm::mat4 sphereTransform;
      sphereTransform = glm::scale(sphereTransform, {3, 3, 3});
      sphereTransform *= glm::lookAt(p1 + 0.1f * spoke_vector, target, up);
      // sphereTransform = glm::translate(sphereTransform, p1 + 0.1f * spoke_vector);
      sphereTransform = glm::scale(sphereTransform, {2, 2, 2});
      sphereMesh.draw(sphereTransform);
    }

    // {
    //   glm::mat4 m;
    //   m = glm::translate(m, {-2.5, 0, 0});
    //   m = glm::scale(m, {1.5, 1.5, 1.5});
    //   m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
    //   m = glm::rotate(m, (float)DEMOLOOP_M_PI, glm::vec3(0, 1, 0));

    //   gl.prepareDraw(m);
    //   gl.drawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    // }

    shader.detach();
  }

private:
  GLuint texture;
  Shader shader;
  Vertex vertices[numVertices];
  glm::vec3 normals[numVertices];
  uint32_t indices[numIndices];

  Mesh sphereMesh;
};

int main(int, char**){
  Loop055 test;
  test.Run();

  return 0;
}
