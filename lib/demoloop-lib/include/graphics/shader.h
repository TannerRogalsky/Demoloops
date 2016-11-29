#pragma once

#include <string>
#include <map>
#include <GL/glew.h>
#include "graphics/gl.h"
#include "common/string_map.h"
#include "glm/glm.hpp"
#include "graphics/canvas.h"

namespace demoloop {
struct ShaderSource
{
  std::string vertex;
  std::string fragment;
};

class Shader {

public:

  // Pointer to currently active Shader.
  static Shader *current;

  // Pointer to the default Shader.
  static Shader *defaultShader;

  // Built-in uniform (extern) variables.
  enum BuiltinUniform
  {
    BUILTIN_TRANSFORM_MATRIX = 0,
    BUILTIN_PROJECTION_MATRIX,
    BUILTIN_TRANSFORM_PROJECTION_MATRIX,
    BUILTIN_MODEL_MATRIX,
    BUILTIN_NORMAL_MATRIX,
    BUILTIN_POINT_SIZE,
    BUILTIN_SCREEN_SIZE,
    BUILTIN_VIDEO_Y_CHANNEL,
    BUILTIN_VIDEO_CB_CHANNEL,
    BUILTIN_VIDEO_CR_CHANNEL,
    BUILTIN_MAX_ENUM
  };

  enum UniformType
  {
    UNIFORM_FLOAT,
    UNIFORM_INT,
    UNIFORM_BOOL,
    UNIFORM_SAMPLER,
    UNIFORM_UNKNOWN,
    UNIFORM_MAX_ENUM
  };

  Shader();
  Shader(const ShaderSource &source);
  ~Shader();

  std::map<std::string, GLint> mAttributes;
  GLint getAttribLocation(const std::string &name);

  /**
   * Binds this Shader's program to be used when rendering.
   *
   * @param temporary True if we just want to send values to the shader with no intention of rendering.
   **/
  void attach();

  /**
   * Detach the currently bound Shader.
   * Causes the GPU rendering pipeline to use fixed functionality in place of shader programs.
   **/
  static void detach();

  /**
   * Send at least one integer or int-vector value to this Shader as a uniform.
   *
   * @param name The name of the uniform variable in the source code.
   * @param size Number of elements in each vector to send.
   *             A value of 1 indicates a single-component vector (an int).
   * @param vec Pointer to the integer or int-vector values.
   * @param count Number of integer or int-vector values.
   **/
  void sendInt(const std::string &name, int size, const GLint *vec, int count);

  /**
   * Send at least one float or vector value to this Shader as a uniform.
   *
   * @param name The name of the uniform variable in the source code.
   * @param size Number of elements in each vector to send.
   *             A value of 1 indicates a single-component vector (a float).
   * @param vec Pointer to the float or float-vector values.
   * @param count Number of float or float-vector values.
   **/
  void sendFloat(const std::string &name, int size, const GLfloat *vec, int count);

  /**
   * Send at least one matrix to this Shader as a uniform.
   *
   * @param name The name of the uniform variable in the source code.
   * @param size Number of rows/columns in the matrix.
   * @param m Pointer to the first element of the first matrix.
   * @param count Number of matrices to send.
   **/
  void sendMatrix(const std::string &name, int size, const GLfloat *m, int count);

  void checkSetScreenParams();
  // void checkSetPointSize(float size);
  void checkSetBuiltinUniforms(const glm::mat4 &curModel);

private:

  struct Uniform
  {
    GLint location;
    GLint count;
    GLenum type;
    UniformType baseType;
    std::string name;
  };

  // Location values for any built-in uniform variables.
  GLint builtinUniforms[BUILTIN_MAX_ENUM];

  // Location values for any generic vertex attribute variables.
  GLint builtinAttributes[ATTRIB_MAX_ENUM];

  const Uniform &getUniform(const std::string &name) const;

  int getUniformTypeSize(GLenum type) const;
  UniformType getUniformBaseType(GLenum type) const;

  // Uniform location buffer map
  std::map<std::string, Uniform> mUniforms;

  // Map active uniform names to their locations.
  void mapActiveUniforms();

  bool loadVolatile(const std::string &vertexShaderSource, const std::string &fragmentShaderSource);

  // Program identifier
  GLuint mProgram;

  glm::mat4 lastTransformMatrix;
  glm::mat4 lastProjectionMatrix;
  glm::mat4 lastModelMatrix;

  // Pointer to the active Canvas when the screen params were last checked.
  Canvas *lastCanvas;
  GL::Viewport lastViewport;

  // Names for the generic vertex attributes.
  static StringMap<VertexAttribID, ATTRIB_MAX_ENUM>::Entry attribNameEntries[];
  static StringMap<VertexAttribID, ATTRIB_MAX_ENUM> attribNames;

  // Names for the built-in uniform variables.
  static StringMap<BuiltinUniform, BUILTIN_MAX_ENUM>::Entry builtinNameEntries[];
  static StringMap<BuiltinUniform, BUILTIN_MAX_ENUM> builtinNames;
};

}
