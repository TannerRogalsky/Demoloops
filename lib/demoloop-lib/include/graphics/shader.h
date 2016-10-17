#pragma once

#include <string>
#include <map>
#include <GL/glew.h>

namespace Demoloop {
struct ShaderSource
{
  std::string vertex;
  std::string fragment;
};

class Shader {

public:

  enum UniformType
  {
    UNIFORM_FLOAT,
    UNIFORM_INT,
    UNIFORM_BOOL,
    UNIFORM_SAMPLER,
    UNIFORM_UNKNOWN,
    UNIFORM_MAX_ENUM
  };

  Shader(const ShaderSource &source);
  ~Shader();
  GLint getAttribLocation(const std::string &name);

  /**
   * Binds this Shader's program to be used when rendering.
   *
   * @param temporary True if we just want to send values to the shader with no intention of rendering.
   **/
  void attach(bool temporary = false);

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

private:

  struct Uniform
  {
    GLint location;
    GLint count;
    GLenum type;
    UniformType baseType;
    std::string name;
  };

  const Uniform &getUniform(const std::string &name) const;

  int getUniformTypeSize(GLenum type) const;
  UniformType getUniformBaseType(GLenum type) const;

  // Uniform location buffer map
  std::map<std::string, Uniform> mUniforms;

  // Map active uniform names to their locations.
  void mapActiveUniforms();

  // Program identifier
  GLuint mProgram;
};

}
