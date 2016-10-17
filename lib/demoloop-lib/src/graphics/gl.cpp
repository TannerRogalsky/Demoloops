#include "graphics/gl.h"
#include <string>
#include <iostream>

const std::string defaultVertexShader = "vec4 position(mat4 transform_proj, vec4 vertpos) {\n"
                                        "  return transform_proj * vertpos;\n"
                                        "}\n";
const std::string defaultFragShader = "vec4 effect(mediump vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) {\n"
                                      "  return Texel(tex, texcoord) * vcolor;\n"
                                      "}\n";

namespace Demoloop {
  GL::GL()
    :mDefaultShader({defaultVertexShader, defaultFragShader})
  {
    mDefaultShader.attach();

    glGenBuffers(1, &mTexCoordBuffer);
    glGenBuffers(1, &mPositionBuffer);
    glGenBuffers(1, &mIndicesBuffer);

    createDefaultTexture();
    initMatrices();

    std::cout << "VertexPosition: " << mDefaultShader.getAttribLocation("VertexPosition") << std::endl;
    // std::cout << "VertexTexCoord: " << mDefaultShader.getAttribLocation("VertexTexCoord") << std::endl;
    // std::cout << "VertexColor: " << mDefaultShader.getAttribLocation("VertexColor") << std::endl;
    std::cout << "ConstantColor: " << mDefaultShader.getAttribLocation("ConstantColor") << std::endl;

    glBindTexture(GL_TEXTURE_2D, mDefaultTexture);
    // glVertexAttrib4f(mDefaultShader.getAttribLocation("VertexColor"), 1, 1, 1, 1);
    glVertexAttrib4f(mDefaultShader.getAttribLocation("ConstantColor"), 1, 1, 1, 1);

    // GLint maxvertexattribs = 1;
    // glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxvertexattribs);
    // std::cout << maxvertexattribs << std::endl;

    // for (int i = 0; i < maxvertexattribs; ++i)
    // {
    //   glDisableVertexAttribArray(i);
    // }
  }

  GL::~GL() {}

  void GL::initMatrices() {
    matrices.transform.clear();
    matrices.projection.clear();

    matrices.transform.push_back(Matrix4());
    matrices.projection.push_back(Matrix4());
  }

  void GL::setViewport(const GL::Viewport &v) {
    glViewport(v.x, v.y, v.w, v.h);
    GLfloat dimensions[4] = {(GLfloat)v.w, (GLfloat)v.h, 0, 0};
    mDefaultShader.attach();
    mDefaultShader.sendFloat("demoloop_ScreenSize", 4, dimensions, 1);
    matrices.projection.back() = Matrix4::ortho(0, v.w, v.h, 0);
  }

  void GL::prepareDraw() {
    mDefaultShader.attach();

    Matrix4 transform = matrices.transform.back();
    // mDefaultShader.sendMatrix("TransformMatrix", 4, transform.getElements(), 1);
    Matrix4 projection = matrices.projection.back();
    // mDefaultShader.sendMatrix("ProjectionMatrix", 4, projection.getElements(), 1);
    Matrix4 tpMatrix(projection * transform);
    mDefaultShader.sendMatrix("TransformProjectionMatrix", 4, tpMatrix.getElements(), 1);
  }

  void GL::polygon(const Vertex *coords, size_t count) {
    prepareDraw();

    glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), &coords[0].x, GL_DYNAMIC_DRAW);

    GLint positionLocation = mDefaultShader.getAttribLocation("VertexPosition");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, count);
    glDisableVertexAttribArray(positionLocation);
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
