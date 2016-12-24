#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/mesh.h"
#include "helpers.h"
#include <array>
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

const float size = 3;

Vertex plane(const float u, const float v) {
  return {
    (u - 0.5f) * size, (v - 0.5f) * size, 0,
    1 - u, 1 - v,
    255, 255, 255, 255
  };
}

Vertex flatMobius(float s, float t) {
  float u = s - 0.5;
  float v = 2 * DEMOLOOP_M_PI * -t;

  float x, y, z;

  float a = size / 2;
  x = cosf(v) * (a + u * cosf(v / 2));
  y = sinf(v) * (a + u * cosf(v / 2));
  z = u * sinf( v / 2 );

  return {
    x, y, z,
    s, (1 - t) * 3
  };
}

Vertex volumetricMobius(float s, float v) {
  float u = s * DEMOLOOP_M_PI;
  float t = v * 2 * DEMOLOOP_M_PI;

  u = u * 2;
  float phi = u / 2;
  float major = size / 2, a = 0.125, b = 0.65;
  float x, y, z;
  x = a * cosf( t ) * cosf( phi ) - b * sinf( t ) * sinf( phi );
  z = a * cosf( t ) * sinf( phi ) + b * sinf( t ) * cosf( phi );
  y = ( major + x ) * sinf( u );
  x = ( major + x ) * cosf( u );

  return {
    x, y, z,
    s, 1 - v
  };
}

Vertex parametricSphere(float s, float t) {
  float u = t * DEMOLOOP_M_PI;
  float v = s * DEMOLOOP_M_PI * 2;
  // v *= 3.0/4.0;

  float radius = size / 2;
  float x = -radius * sinf(u) * sinf(v);
  float y = -radius * cosf(u);
  float z = radius * sinf(u) * cosf(v);

  return {
    x, y, z,
    1 - s, 1 - t
  };
}

template <
  typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
> T mix(T const& a, T const& b, const float& ratio)  {
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

const array<function<Vertex(float, float)>, 2> surfaces = {{
  plane, parametricSphere//, volumetricMobius, flatMobius
}};

const uint32_t stacks = 30, slices = 30;
const uint32_t numVertices = (slices + 1) * (stacks + 1);
const uint32_t numIndices = slices * stacks * 6;

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(150, 150, 150) {
    // glEnable(GL_CULL_FACE);
    texture = loadTexture("uv_texture.jpg");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  }

  ~Loop055() {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;
    const float mod_ratio = powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2);
    const uint32_t surfaceIndex = fmod(floor(t / CYCLE_LENGTH), surfaces.size());

    const uint32_t sliceCount = slices + 1;

    uint32_t index = 0;
    for (uint32_t i = 0; i <= stacks; ++i) {
      const float v = static_cast<float>(i) / stacks;

      for (uint32_t j = 0; j <= slices; ++j) {
        const float u = static_cast<float>(j) / slices;

        // vertices[index] = parametricSphere(u, v);
        // vertices[index] = mix(plane(u, v), parametricSphere(u, v), mod_ratio);
        vertices[index] = mix(
          surfaces[surfaceIndex](u, v),
          surfaces[(surfaceIndex + 1) % surfaces.size()](u, v),
          cycle_ratio
        );
        normals[index] = glm::vec3(0, 0, 0);
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

    // const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 10), static_cast<float>(cycle_ratio * DEMOLOOP_M_PI * 2), glm::vec3(0, 1, 0));
    const glm::vec3 eye = glm::vec3(0, 0, 10);
    const glm::vec3 target = {0, 0, 0};
    const glm::vec3 up = {0, 1, 0};
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    GL::TempProjection p1(gl);
    p1.get() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

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
      m = glm::translate(m, {2.5, -0.5, 0});
      m = glm::scale(m, {1.5, 1.5, 1.5});
      m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));

      gl.prepareDraw(m);
      gl.drawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    }

    {
      glm::mat4 m;
      m = glm::translate(m, {-2.5, -0.5, 0});
      m = glm::scale(m, {1.5, 1.5, 1.5});
      m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
      m = glm::rotate(m, (float)DEMOLOOP_M_PI, glm::vec3(0, 1, 0));

      gl.prepareDraw(m);
      gl.drawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    }
  }

private:
  GLuint texture;
  Vertex vertices[numVertices];
  glm::vec3 normals[numVertices];
  uint32_t indices[numIndices];
};

int main(int, char**){
  Loop055 test;
  test.Run();

  return 0;
}
