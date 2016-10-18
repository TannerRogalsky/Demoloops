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
  GL::GL() {
    initMatrices();
  }

  GL::~GL() {}

  bool GL::initContext() {
    Shader::defaultShader = new Shader({defaultVertexShader, defaultFragShader});
    Shader::defaultShader->attach();

    glGenBuffers(1, &mTexCoordBuffer);
    glGenBuffers(1, &mPositionBuffer);
    glGenBuffers(1, &mIndicesBuffer);

    createDefaultTexture();

    glBindTexture(GL_TEXTURE_2D, mDefaultTexture);
    glVertexAttrib4f(Shader::defaultShader->getAttribLocation("ConstantColor"), 1, 1, 1, 1);

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
    Shader::defaultShader->attach();
    Shader::defaultShader->sendFloat("demoloop_ScreenSize", 4, dimensions, 1);
    matrices.projection.back() = Matrix4::ortho(0, v.w, v.h, 0);
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
    // Matrix4 transform = matrices.transform.back();
    // Shader::defaultShader->sendMatrix("TransformMatrix", 4, transform.getElements(), 1);
    // Matrix4 projection = matrices.projection.back();
    // Shader::defaultShader->sendMatrix("ProjectionMatrix", 4, projection.getElements(), 1);
    // Matrix4 tpMatrix(projection * transform);
    // Shader::defaultShader->sendMatrix("TransformProjectionMatrix", 4, tpMatrix.getElements(), 1);

    // glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) 640 / (float) 480, 0.1f, 100.0f);
    // // glm::mat4 Projection = glm::ortho(0.0, 640.0, 480.0, 0.0, 0.1, 100.0);
    // // glm::mat4 Projection = glm::ortho(-1.0, 1.0, -1.0, 1.0, 0.1, 100.0);
    // glm::mat4 View = glm::lookAt(
    //     glm::vec3(0,0,100), // Camera is at (4,3,3), in World Space
    //     glm::vec3(0,0,0), // and looks at the origin
    //     glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    //     );
    // // for (int y = 0; y < 4; ++y)
    // // {
    // //   for (int x = 0; x < 4; ++x)
    // //   {
    // //     std::cout << View[y][x] << std::endl;
    // //   }
    // //   std::cout << std::endl;
    // // }
    // // std::cout << std::endl << std::endl;
    // glm::mat4 Model = glm::mat4(1.0f);
    // // Model = translate(Model, glm::vec3(100, 100, 0));
    // glm::mat4 mvp = Projection * View * Model;
    // Shader::defaultShader->sendMatrix("TransformProjectionMatrix", 4, &mvp[0][0], 1);

    Matrix4 View = matrices.transform.back();
    Matrix4 Projection = matrices.projection.back();
    // Matrix4 Projection = Matrix4::perspective(45.0f, 640.0 / 480.0, 0.1, 100.0);

    // for (int i = 0; i < 16; ++i)
    // {
    //   std::cout << View.e[i] << std::endl;
    // }
    // std::cout << std::endl;

    Matrix4 mvp = Projection * View; // * Model
    Shader::defaultShader->sendMatrix("TransformProjectionMatrix", 4, mvp.getElements(), 1);
  }

  void GL::polygon(const Vertex *coords, size_t count) {
    prepareDraw();

    glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), &coords[0].x, GL_DYNAMIC_DRAW);

    GLint positionLocation = Shader::defaultShader->getAttribLocation("VertexPosition");
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
