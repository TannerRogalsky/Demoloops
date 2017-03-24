#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/shader.h"
#include "helpers.h"
#include "graphics/shader.h"
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

const static std::string shaderCode = R"===(
uniform mediump float uTime;

#ifdef VERTEX
uniform sampler2D _tex0_;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vec4 tex = texture2D(_tex0_, VertexTexCoord.st);
  float scrollSpeed = 75.;
  vec4 tex2 = texture2D(_tex0_, fract(vec2(tex.r + uTime * scrollSpeed, tex.b + (uTime * scrollSpeed * 1.234))));

  return transform_proj * model * vec4(tex2.rgb - vec3(.5, .5, .5), 1.);
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  vec4 tex = texture2D(texture, fract(tc + uTime * 250.));

  float a = .13;
  // return vec4(tex.rgb * 1.5, a + (1.0 - pow(sin(uTime * 5000), 2.0) * a));
  return vec4(tex.rgb * 1.5, a);
}
#endif
)===";

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(720, 720, 0, 0, 0), shader({shaderCode, shaderCode}), planeMesh(plane(10, 10, 256, 256)), offset(rand()) {
    // glDisable(GL_DEPTH_TEST);
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    noiseTexture = loadTexture("loop064/rgb-perlin-seamless-512.png");

    gl.bufferVertices(planeMesh.mVertices.data(), planeMesh.mVertices.size(), GL_STATIC_DRAW);
    gl.bufferIndices(planeMesh.mIndices.data(), planeMesh.mIndices.size(), GL_STATIC_DRAW);

    gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));
  }

  ~Loop055() {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    // const float eyeRot = 0;
    const float eyeRot = cycle_ratio * DEMOLOOP_M_PI * 2;

    const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 1), eyeRot, glm::vec3(-0.3, 1, 0));
    // const glm::vec3 eye = glm::rotate(glm::vec3(0, 4, 40), eyeRot, glm::vec3(-0.3, 1, 0));
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    shader.attach();
    {
      float uTime = powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2) / 350 + offset / static_cast<float>(RAND_MAX);
      shader.sendFloat("uTime", 1, &uTime, 1);
    }

    gl.prepareDraw();

    // gl.drawElements(GL_TRIANGLES, planeMesh.mIndices.size(), GL_UNSIGNED_INT, 0);
    gl.drawElements(GL_LINE_STRIP, planeMesh.mIndices.size(), GL_UNSIGNED_INT, 0);

    shader.detach();
  }

private:
  Shader shader;
  Mesh planeMesh;
  GLuint noiseTexture;
  const int offset;
};

int main(int, char**){
  srand(time(0)); rand();

  Loop055 test;
  test.Run();

  return 0;
}
