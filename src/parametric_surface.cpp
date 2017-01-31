#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/mesh.h"
#include "helpers.h"
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

const float size = 1;

Vertex plane(const float u, const float v) {
  return {
    u * size, v * size, 0,
    u, 1 - v,
    255, 255, 255, 255
  };
}

Vertex flatMobius(float s, float t) {
  float u = s - 0.5;
  float v = 2 * DEMOLOOP_M_PI * t;

  float x, y, z;

  float a = 2;
  x = cosf(v) * (a + u * cosf(v / 2));
  y = sinf(v) * (a + u * cosf(v / 2));
  z = u * sinf( v / 2 );

  return {
    x, y, z,
    s, t
  };
}

Vertex volumetricMobius(float s, float v) {
  float u = s * DEMOLOOP_M_PI;
  float t = v * 2 * DEMOLOOP_M_PI;

  u = u * 2;
  float phi = u / 2;
  float major = 2.25, a = 0.125, b = 0.65;
  float x, y, z;
  x = a * cosf( t ) * cosf( phi ) - b * sinf( t ) * sinf( phi );
  z = a * cosf( t ) * sinf( phi ) + b * sinf( t ) * cosf( phi );
  y = ( major + x ) * sinf( u );
  x = ( major + x ) * cosf( u );

  return {
    x, y, z,
    s, v
  };
}

template <
  typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
> T mix(T const& a, T const& b, const float& ratio)  {
  return a * (1.0f - ratio) + b * ratio;
}

Vertex mix(const Vertex& a, const Vertex& b, const float& ratio) {
  const float x = mix<float>(a.x, b.x, ratio);
  const float y = mix<float>(a.y, b.y, ratio);
  const float z = mix<float>(a.z, b.z, ratio);

  const float s = mix<float>(a.s, b.s, ratio);
  const float t = mix<float>(a.t, b.t, ratio);

  return {x, y, z, s, t};
}

class Test4 : public Demoloop {
public:
  Test4() : Demoloop(150, 150, 150), mesh(parametric(volumetricMobius, 30, 30)) {
    // glEnable(GL_CULL_FACE);
    texture = loadTexture("uv_texture.jpg");
  }

  ~Test4() {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    {
      // const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 4), static_cast<float>(-cycle_ratio * DEMOLOOP_M_PI * 2), glm::vec3(0, 1, 0));
      const glm::vec3 eye = glm::vec3(0, 0, 8);
      const glm::vec3 target = {0, 0, 0};
      const glm::vec3 up = {0, 1, 0};
      glm::mat4 camera = glm::lookAt(eye, target, up);

      GL::TempTransform t1(gl);
      t1.get() = camera;

      GL::TempProjection p1(gl);
      p1.get() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

      glm::mat4 m;
      m = glm::translate(m, {0, -size / 2, 0});
      m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
      // m = glm::rotate(m, (float)-DEMOLOOP_M_PI / 2, glm::vec3(0, 1, 0));
      // m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(1, 0, 0));

      t1.get() *= m;
      gl.bindTexture(texture);
      // gl.triangles(mesh.mVertices.data(), mesh.mVertices.size());

      gl.prepareDraw();

      gl.bufferVertices(mesh.mVertices.data(), mesh.mVertices.size());
      gl.bufferIndices(mesh.mIndices.data(), mesh.mIndices.size());

      gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD);
      glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
      glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
      glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

      gl.drawElements(GL_TRIANGLES, mesh.mIndices.size(), GL_UNSIGNED_INT, 0);

      // mesh.draw(m);
    }

    // renderTexture(texture, 0, 0, 1, height, height);
  }

private:
  Mesh mesh;
  GLuint texture;
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
