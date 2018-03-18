#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/mesh.h"
#include "helpers.h"
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;


const float CYCLE_LENGTH = 10;

class Test4 : public Demoloop {
public:
  Test4() : Demoloop(CYCLE_LENGTH, 1280, 640, 150, 150, 150), mesh(sphere(1, 30, 30)) {
    // glEnable(GL_CULL_FACE);
    texture = loadTexture("uv_texture.jpg");
  }

  ~Test4() {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    {
      // const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 4), static_cast<float>(-cycle_ratio * DEMOLOOP_M_PI * 2), glm::vec3(0, 1, 0));
      const glm::vec3 eye = glm::vec3(0, 0, 4);
      const glm::vec3 target = {0, 0, 0};
      const glm::vec3 up = {0, 1, 0};
      glm::mat4 camera = glm::lookAt(eye, target, up);

      GL::TempTransform t1(gl);
      t1.get() = camera;

      GL::TempProjection p1(gl);
      p1.get() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

      glm::mat4 m;
      m = glm::translate(m, {1, 0, 0});
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

    renderTexture(texture, 0, 0, 1, height, height);
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
