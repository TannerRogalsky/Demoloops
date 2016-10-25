#include "graphics/gl.h"
#include <string>
#include <iostream>
#include <cstddef>

const std::string defaultVertexShader = "vec4 position(mat4 transform_proj, vec4 vertpos) {\n"
                                        "  return transform_proj * vertpos;\n"
                                        "}\n";
const std::string defaultFragShader = "vec4 effect(mediump vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) {\n"
                                      "  return Texel(tex, texcoord) * vcolor;\n"
                                      "}\n";

namespace Demoloop {
  GL::GL() {
    initMatrices();
  }

  GL::~GL() {}

  bool GL::initContext() {
    glEnable(GL_MULTISAMPLE); // TODO: is this doing anything?
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    Shader::defaultShader = new Shader({defaultVertexShader, defaultFragShader});
    Shader::defaultShader->attach();

    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mIBO);

    createDefaultTexture();

    glBindTexture(GL_TEXTURE_2D, mDefaultTexture);
    glVertexAttrib4f(Shader::defaultShader->getAttribLocation("ConstantColor"), 1, 1, 1, 1);
    glVertexAttrib4f(Shader::defaultShader->getAttribLocation("VertexColor"), 1, 1, 1, 1);

    return true;
  }

  void GL::initMatrices() {
    matrices.transform.clear();
    matrices.projection.clear();

    matrices.transform.push_back(Matrix4());
    matrices.projection.push_back(Matrix4());
  }

  void GL::setViewport(const GL::Viewport &v) {
    glViewport(v.x, v.y, v.w, v.h);
    GLfloat dimensions[4] = {(GLfloat)v.w, (GLfloat)v.h, 0, 0};
    Shader::defaultShader->sendFloat("demoloop_ScreenSize", 4, dimensions, 1);
    matrices.projection.back() = Matrix4::ortho(0, v.w, v.h, 0, 0.1, 100);
    matrices.transform.back() = Matrix4::lookAt({0, 0, 100}, {0, 0, 0}, {0, 1, 0});
  }

  void GL::pushTransform()
  {
    matrices.transform.push_back(matrices.transform.back());
  }

  void GL::popTransform()
  {
    matrices.transform.pop_back();
  }

  Matrix4 &GL::getTransform()
  {
    return matrices.transform.back();
  }

  void GL::pushProjection()
  {
    matrices.projection.push_back(matrices.projection.back());
  }

  void GL::popProjection()
  {
    matrices.projection.pop_back();
  }

  Matrix4 &GL::getProjection()
  {
    return matrices.projection.back();
  }

  void GL::prepareDraw() {
    Matrix4 View = matrices.transform.back();
    Matrix4 Projection = matrices.projection.back();
    Matrix4 mvp = Projection * View; // * Model
    Shader::defaultShader->sendMatrix("TransformProjectionMatrix", 4, mvp.getElements(), 1);
  }

  void GL::lines(const Vertex *coords, size_t count) {
    prepareDraw();

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), &coords[0].x, GL_DYNAMIC_DRAW);

    GLint positionLocation = Shader::defaultShader->getAttribLocation("VertexPosition");
    GLint colorLocation = Shader::defaultShader->getAttribLocation("VertexColor");

    glEnableVertexAttribArray(positionLocation);
    glEnableVertexAttribArray(colorLocation);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glDrawArrays(GL_LINES, 0, count);

    glDisableVertexAttribArray(positionLocation);
    glDisableVertexAttribArray(colorLocation);
  }

  void GL::triangles(const Vertex *coords, size_t count) {
    prepareDraw();

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), &coords[0].x, GL_DYNAMIC_DRAW);

    GLint positionLocation = Shader::defaultShader->getAttribLocation("VertexPosition");
    GLint colorLocation = Shader::defaultShader->getAttribLocation("VertexColor");

    glEnableVertexAttribArray(positionLocation);
    glEnableVertexAttribArray(colorLocation);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glDrawArrays(GL_TRIANGLES, 0, count);

    glDisableVertexAttribArray(positionLocation);
    glDisableVertexAttribArray(colorLocation);
  }

  void GL::triangles(const Triangle *triangleVertices, size_t count) {
    triangles((Vertex *)triangleVertices, count * 3);
  }

  void GL::triangles(const Vertex *coords, const uint32_t *indices, size_t count) {
    prepareDraw();

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), &coords[0], GL_DYNAMIC_DRAW);

    GLint positionLocation = Shader::defaultShader->getAttribLocation("VertexPosition");
    GLint colorLocation = Shader::defaultShader->getAttribLocation("VertexColor");

    glEnableVertexAttribArray(positionLocation);
    glEnableVertexAttribArray(colorLocation);

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), &indices[0], GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(positionLocation);
    glDisableVertexAttribArray(colorLocation);
  }

  void GL::createDefaultTexture()
  {
    // Set the 'default' texture (id 0) as a repeating white pixel. Otherwise,
    // texture2D calls inside a shader would return black when drawing graphics
    // primitives, which would create the need to use different "passthrough"
    // shaders for untextured primitives vs images.

    // GLuint curtexture = state.boundTextures[state.curTextureUnit];

    glGenTextures(1, &mDefaultTexture);
    glBindTexture(GL_TEXTURE_2D, mDefaultTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLubyte pix[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);

    // bindTexture(curtexture);
  }
}
