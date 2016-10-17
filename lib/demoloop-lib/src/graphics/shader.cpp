#include "graphics/shader.h"
#include "opengl_helpers.h"
#include <iostream>

namespace Demoloop {

Shader *Shader::current = nullptr;
Shader *Shader::defaultShader = nullptr;

Shader::Shader(const ShaderSource &source) {
  mProgram = loadProgram(createVertexCode(source.vertex), createFragmentCode(source.fragment));
  mapActiveUniforms();

  // std::cout << "Shader Source:" << std::endl
  //           << createVertexCode(source.vertex) << std::endl
  //           << createFragmentCode(source.fragment) << std::endl << std::endl;

  // std::cout << "Uniforms:" << std::endl;
  // for (auto it = mUniforms.begin(); it != mUniforms.end(); ++it) { // calls a_map.begin() and a_map.end()
  //   std::cout << it->first << ", " << it->second.location << '\n';
  // }
}

Shader::~Shader() {}

GLint Shader::getAttribLocation(const std::string &name) {
  GLint location = glGetAttribLocation(mProgram, name.c_str());
  return location;
}

void Shader::mapActiveUniforms()
{
  // Built-in uniform locations default to -1 (nonexistant.)
  // for (int i = 0; i < int(BUILTIN_MAX_ENUM); i++)
  //   builtinUniforms[i] = -1;

  mUniforms.clear();

  GLint activeprogram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &activeprogram);

  glUseProgram(mProgram);

  GLint numuniforms;
  glGetProgramiv(mProgram, GL_ACTIVE_UNIFORMS, &numuniforms);

  GLchar cname[256];
  const GLint bufsize = (GLint) (sizeof(cname) / sizeof(GLchar));

  for (int i = 0; i < numuniforms; i++)
  {
    GLsizei namelen = 0;
    Uniform u = {};

    glGetActiveUniform(mProgram, (GLuint) i, bufsize, &namelen, &u.count, &u.type, cname);

    u.name = std::string(cname, (size_t) namelen);
    u.location = glGetUniformLocation(mProgram, u.name.c_str());
    u.baseType = getUniformBaseType(u.type);

    // Initialize all samplers to 0. Both GLSL and GLSL ES are supposed to
    // do this themselves, but some Android devices (galaxy tab 3 and 4)
    // don't seem to do it...
    if (u.baseType == UNIFORM_SAMPLER)
      glUniform1i(u.location, 0);

    // glGetActiveUniform appends "[0]" to the end of array uniform names...
    if (u.name.length() > 3)
    {
      size_t findpos = u.name.find("[0]");
      if (findpos != std::string::npos && findpos == u.name.length() - 3)
        u.name.erase(u.name.length() - 3);
    }

    // // If this is a built-in (LOVE-created) uniform, store the location.
    // BuiltinUniform builtin;
    // if (builtinNames.find(u.name.c_str(), builtin))
    //   builtinUniforms[int(builtin)] = u.location;

    if (u.location != -1)
      mUniforms[u.name] = u;
  }

  glUseProgram(activeprogram);
}

const Shader::Uniform &Shader::getUniform(const std::string &name) const
{
  std::map<std::string, Uniform>::const_iterator it = mUniforms.find(name);

  if (it == mUniforms.end()) {
    throw std::invalid_argument("Variable does not exist.\n"
                          "A common error is to define but not use the variable: " + name);
  }

  return it->second;
}

void Shader::attach(bool temporary) {
  glUseProgram(mProgram);
  // if (current != this)
  // {
  //   glUseProgram(program);
  //   current = this;
  //   // retain/release happens in Graphics::setShader.
  // }

  // if (!temporary)
  // {
  //   // make sure all sent textures are properly bound to their respective texture units
  //   // note: list potentially contains texture ids of deleted/invalid textures!
  //   for (size_t i = 0; i < activeTexUnits.size(); ++i)
  //   {
  //     if (activeTexUnits[i] > 0)
  //       gl.bindTextureToUnit(activeTexUnits[i], i + 1, false);
  //   }

  //   // We always want to use texture unit 0 for everyhing else.
  //   gl.setTextureUnit(0);
  // }
}

void Shader::detach() {
  // if (defaultShader)
  // {
  //   if (current != defaultShader)
  //     defaultShader->attach();

  //   return;
  // }

  // if (current != nullptr)
    glUseProgram(0);

  // current = nullptr;
}

void Shader::sendInt(const std::string &name, int size, const GLint *vec, int count)
{
  // TemporaryAttacher attacher(this);

  const Uniform &u = getUniform(name);
  // checkSetUniformError(u, size, count, UNIFORM_INT);

  switch (size)
  {
  case 4:
    glUniform4iv(u.location, count, vec);
    break;
  case 3:
    glUniform3iv(u.location, count, vec);
    break;
  case 2:
    glUniform2iv(u.location, count, vec);
    break;
  case 1:
  default:
    glUniform1iv(u.location, count, vec);
    break;
  }
}

void Shader::sendFloat(const std::string &name, int size, const GLfloat *vec, int count)
{
  // TemporaryAttacher attacher(this);

  const Uniform &u = getUniform(name);
  // checkSetUniformError(u, size, count, UNIFORM_FLOAT);

  switch (size)
  {
  case 4:
    glUniform4fv(u.location, count, vec);
    break;
  case 3:
    glUniform3fv(u.location, count, vec);
    break;
  case 2:
    glUniform2fv(u.location, count, vec);
    break;
  case 1:
  default:
    glUniform1fv(u.location, count, vec);
    break;
  }
}

void Shader::sendMatrix(const std::string &name, int size, const GLfloat *m, int count)
{
  // TemporaryAttacher attacher(this);

  // if (size < 2 || size > 4)
  // {
  //   throw love::Exception("Invalid matrix size: %dx%d "
  //               "(can only set 2x2, 3x3 or 4x4 matrices.)", size,size);
  // }

  const Uniform &u = getUniform(name);
  // checkSetUniformError(u, size, count, UNIFORM_FLOAT);

  switch (size)
  {
  case 4:
    glUniformMatrix4fv(u.location, count, GL_FALSE, m);
    break;
  case 3:
    glUniformMatrix3fv(u.location, count, GL_FALSE, m);
    break;
  case 2:
  default:
    glUniformMatrix2fv(u.location, count, GL_FALSE, m);
    break;
  }
}

int Shader::getUniformTypeSize(GLenum type) const {
  switch (type)
  {
  case GL_INT:
  case GL_FLOAT:
  case GL_BOOL:
  case GL_SAMPLER_1D:
  case GL_SAMPLER_2D:
  case GL_SAMPLER_3D:
    return 1;
  case GL_INT_VEC2:
  case GL_FLOAT_VEC2:
  case GL_FLOAT_MAT2:
  case GL_BOOL_VEC2:
    return 2;
  case GL_INT_VEC3:
  case GL_FLOAT_VEC3:
  case GL_FLOAT_MAT3:
  case GL_BOOL_VEC3:
    return 3;
  case GL_INT_VEC4:
  case GL_FLOAT_VEC4:
  case GL_FLOAT_MAT4:
  case GL_BOOL_VEC4:
    return 4;
  default:
    return 1;
  }
}

Shader::UniformType Shader::getUniformBaseType(GLenum type) const {
  switch (type)
  {
  case GL_INT:
  case GL_INT_VEC2:
  case GL_INT_VEC3:
  case GL_INT_VEC4:
    return UNIFORM_INT;
  case GL_FLOAT:
  case GL_FLOAT_VEC2:
  case GL_FLOAT_VEC3:
  case GL_FLOAT_VEC4:
  case GL_FLOAT_MAT2:
  case GL_FLOAT_MAT3:
  case GL_FLOAT_MAT4:
  case GL_FLOAT_MAT2x3:
  case GL_FLOAT_MAT2x4:
  case GL_FLOAT_MAT3x2:
  case GL_FLOAT_MAT3x4:
  case GL_FLOAT_MAT4x2:
  case GL_FLOAT_MAT4x3:
    return UNIFORM_FLOAT;
  case GL_BOOL:
  case GL_BOOL_VEC2:
  case GL_BOOL_VEC3:
  case GL_BOOL_VEC4:
    return UNIFORM_BOOL;
  case GL_SAMPLER_1D:
  case GL_SAMPLER_1D_SHADOW:
  case GL_SAMPLER_1D_ARRAY:
  case GL_SAMPLER_1D_ARRAY_SHADOW:
  case GL_SAMPLER_2D:
  case GL_SAMPLER_2D_MULTISAMPLE:
  case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
  case GL_SAMPLER_2D_RECT:
  case GL_SAMPLER_2D_RECT_SHADOW:
  case GL_SAMPLER_2D_SHADOW:
  case GL_SAMPLER_2D_ARRAY:
  case GL_SAMPLER_2D_ARRAY_SHADOW:
  case GL_SAMPLER_3D:
  case GL_SAMPLER_CUBE:
  case GL_SAMPLER_CUBE_SHADOW:
  case GL_SAMPLER_CUBE_MAP_ARRAY:
  case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
    return UNIFORM_SAMPLER;
  default:
    return UNIFORM_UNKNOWN;
  }
}

}
