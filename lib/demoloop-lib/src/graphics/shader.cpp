#include "graphics/shader.h"
#include "opengl_helpers.h"
#include <limits>
#include <cstring> // memcpy
#include <glm/gtc/matrix_transform.hpp>

const static std::string defaultVertexShader = "vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {\n"
                                               "  return transform_proj * model * vertpos;\n"
                                               "}\n";
const static std::string defaultFragShader = "vec4 effect(mediump vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) {\n"
                                             "  return Texel(tex, texcoord) * vcolor;\n"
                                             "}\n";

namespace demoloop {

namespace
{
  // temporarily attaches a shader program (for setting uniforms, etc)
  // reattaches the originally active program when destroyed
  struct TemporaryAttacher
  {
    TemporaryAttacher(Shader *shader)
    : curShader(shader)
    , prevShader(Shader::current)
    {
      // curShader->attach(true);
      curShader->attach();
    }

    ~TemporaryAttacher()
    {
      if (prevShader != nullptr)
        prevShader->attach();
      else
        curShader->detach();
    }

    Shader *curShader;
    Shader *prevShader;
  };
} // anonymous namespace

Shader *Shader::current = nullptr;
Shader *Shader::defaultShader = nullptr;

Shader::Shader() : Shader({defaultVertexShader, defaultFragShader}) {}

Shader::Shader(const ShaderSource &source) {
  loadVolatile(createVertexCode(source.vertex), createFragmentCode(source.fragment));
  mapActiveUniforms();

  for (int i = 0; i < int(ATTRIB_MAX_ENUM); i++)
  {
    const char *name = nullptr;
    if (attribNames.find(VertexAttribID(i), name))
      builtinAttributes[i] = glGetAttribLocation(mProgram, name);
    else
      builtinAttributes[i] = -1;
  }

  // Invalidate the cached matrices by setting some elements to NaN.
  float nan = std::numeric_limits<float>::quiet_NaN();
  lastProjectionMatrix = glm::translate(lastProjectionMatrix, {nan, nan, nan});
  lastTransformMatrix = glm::translate(lastTransformMatrix, {nan, nan, nan});
  lastModelMatrix = glm::translate(lastModelMatrix, {nan, nan, nan});
}

Shader::~Shader() {}

GLint Shader::getAttribLocation(const std::string &name) {
  auto it = mAttributes.find(name);
  if (it != mAttributes.end())
    return it->second;

  GLint location = glGetAttribLocation(mProgram, name.c_str());

  mAttributes[name] = location;
  return location;
}

void Shader::mapActiveUniforms()
{
  // Built-in uniform locations default to -1 (nonexistant.)
  for (int i = 0; i < int(BUILTIN_MAX_ENUM); i++)
    builtinUniforms[i] = -1;

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
    BuiltinUniform builtin;
    if (builtinNames.find(u.name.c_str(), builtin))
      builtinUniforms[int(builtin)] = u.location;

    if (u.location != -1)
      mUniforms[u.name] = u;
  }

  glUseProgram(activeprogram);
}

bool Shader::loadVolatile(const std::string &vertexShaderSource, const std::string &fragmentShaderSource) {
  mProgram = glCreateProgram();

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const char *vertexSrc = vertexShaderSource.c_str();
  GLint vertexSrclen = (GLint) vertexShaderSource.length();
  glShaderSource(vertexShader, 1, (const GLchar **)&vertexSrc, &vertexSrclen);
  glCompileShader(vertexShader);
  GLint vShaderCompiled = GL_FALSE;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
  if (vShaderCompiled != GL_TRUE) {
    printShaderLog(vertexShader);
    return false;
  } else {
    glAttachShader(mProgram, vertexShader);
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fragmentSrc = fragmentShaderSource.c_str();
  GLint fragmentSrclen = (GLint) fragmentShaderSource.length();
  glShaderSource(fragmentShader, 1, (const GLchar **)&fragmentSrc, &fragmentSrclen);
  glCompileShader(fragmentShader);
  GLint fShaderCompiled = GL_FALSE;
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
  if(fShaderCompiled != GL_TRUE) {
    printShaderLog(fragmentShader);
    return false;
  } else {
    glAttachShader(mProgram, fragmentShader);
  }

  // Bind generic vertex attribute indices to names in the shader.
  for (int i = 0; i < int(ATTRIB_MAX_ENUM); i++)
  {
    const char *name = nullptr;
    if (attribNames.find((VertexAttribID) i, name))
      glBindAttribLocation(mProgram, i, (const GLchar *) name);
  }

  glLinkProgram(mProgram);

  //Check for errors
  GLint programSuccess = GL_TRUE;
  glGetProgramiv(mProgram, GL_LINK_STATUS, &programSuccess);
  if(programSuccess != GL_TRUE) {
    printProgramLog(mProgram);
    return false;
  }

  glValidateProgram(mProgram);

  GLint programValid = GL_TRUE;
  glGetProgramiv(mProgram, GL_VALIDATE_STATUS, &programValid);
  if(programValid != GL_TRUE) {
    printProgramLog(mProgram);
    return false;
  }

  return true;
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

void Shader::attach() {
  if (current != this)
  {
    glUseProgram(mProgram);
    current = this;
    // retain/release happens in Graphics::setShader.
  }

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
  if (defaultShader) {
    if (current != defaultShader)
      defaultShader->attach();

    return;
  }

  if (current != nullptr)
    glUseProgram(0);

  current = nullptr;
}

void Shader::checkSetScreenParams()
{
  GL::Viewport view = gl.getViewport();

  if (view == lastViewport && lastCanvas == Canvas::current)
    return;

  // In the shader, we do pixcoord.y = gl_FragCoord.y * params.z + params.w.
  // This lets us flip pixcoord.y when needed, to be consistent (drawing with
  // no Canvas active makes the y-values for pixel coordinates flipped.)
  GLfloat params[] = {
    (GLfloat) view.w, (GLfloat) view.h,
    0.0f, 0.0f,
  };

  if (Canvas::current != nullptr) {
    // No flipping: pixcoord.y = gl_FragCoord.y * 1.0 + 0.0.
    params[2] = 1.0f;
    params[3] = 0.0f;
  } else {
    // gl_FragCoord.y is flipped when drawing to the screen, so we un-flip:
    // pixcoord.y = gl_FragCoord.y * -1.0 + height.
    params[2] = -1.0f;
    params[3] = (GLfloat) view.h;
  }

  GLint location = builtinUniforms[BUILTIN_SCREEN_SIZE];

  if (location >= 0) {
    TemporaryAttacher attacher(this);
    glUniform4fv(location, 1, params);
  }

  lastCanvas = Canvas::current;
  lastViewport = view;
}

void Shader::checkSetBuiltinUniforms(const glm::mat4 &curModel)
{
  checkSetScreenParams();
  // checkSetPointSize(gl.getPointSize());

  const glm::mat4 &curxform = gl.matrices.transform.back();
  const glm::mat4 &curproj = gl.matrices.projection.back();

  TemporaryAttacher attacher(this);

  bool tpmatrixneedsupdate = false;

  if (curModel != lastModelMatrix) {
    GLint location = builtinUniforms[BUILTIN_MODEL_MATRIX];
    if (location >= 0) {
      glUniformMatrix4fv(location, 1, GL_FALSE, &curModel[0][0]);
    }

    // Also upload the re-calculated normal matrix, if possible. The
    // normal matrix is the transpose of the inverse of the rotation
    // portion (top-left 3x3) of the transform matrix.
    location = builtinUniforms[BUILTIN_NORMAL_MATRIX];
    if (location >= 0)
    {
      glm::mat3 normalmatrix = glm::inverse(glm::transpose(curModel));
      glUniformMatrix3fv(location, 1, GL_FALSE, &normalmatrix[0][0]);
    }

    lastModelMatrix = curModel;
  }

  // Only upload the matrices if they've changed.
  if (curxform != lastTransformMatrix) {
    GLint location = builtinUniforms[BUILTIN_TRANSFORM_MATRIX];
    if (location >= 0)
      glUniformMatrix4fv(location, 1, GL_FALSE, &curxform[0][0]);

    tpmatrixneedsupdate = true;
    lastTransformMatrix = curxform;
  }

  if (curproj != lastProjectionMatrix)
  {
    GLint location = builtinUniforms[BUILTIN_PROJECTION_MATRIX];
    if (location >= 0)
      glUniformMatrix4fv(location, 1, GL_FALSE, &curproj[0][0]);

    tpmatrixneedsupdate = true;
    lastProjectionMatrix = curproj;
  }

  if (tpmatrixneedsupdate)
  {
    GLint location = builtinUniforms[BUILTIN_TRANSFORM_PROJECTION_MATRIX];
    if (location >= 0)
    {
      glm::mat4 tp_matrix(curproj * curxform);
      glUniformMatrix4fv(location, 1, GL_FALSE, &tp_matrix[0][0]);
    }
  }
}

void Shader::sendInt(const std::string &name, int size, const GLint *vec, int count)
{
  TemporaryAttacher attacher(this);

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
  TemporaryAttacher attacher(this);

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
  TemporaryAttacher attacher(this);

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

StringMap<VertexAttribID, ATTRIB_MAX_ENUM>::Entry Shader::attribNameEntries[] =
{
  {"VertexPosition", ATTRIB_POS},
  {"VertexTexCoord", ATTRIB_TEXCOORD},
  {"VertexColor", ATTRIB_COLOR},
  {"ConstantColor", ATTRIB_CONSTANTCOLOR},
};

StringMap<VertexAttribID, ATTRIB_MAX_ENUM> Shader::attribNames(Shader::attribNameEntries, sizeof(Shader::attribNameEntries));

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM>::Entry Shader::builtinNameEntries[] =
{
  {"TransformMatrix", Shader::BUILTIN_TRANSFORM_MATRIX},
  {"ProjectionMatrix", Shader::BUILTIN_PROJECTION_MATRIX},
  {"TransformProjectionMatrix", Shader::BUILTIN_TRANSFORM_PROJECTION_MATRIX},
  {"ModelMatrix", Shader::BUILTIN_MODEL_MATRIX},
  {"NormalMatrix", Shader::BUILTIN_NORMAL_MATRIX},
  {"demoloop_PointSize", Shader::BUILTIN_POINT_SIZE},
  {"demoloop_ScreenSize", Shader::BUILTIN_SCREEN_SIZE},
  {"demoloop_VideoYChannel", Shader::BUILTIN_VIDEO_Y_CHANNEL},
  {"demoloop_VideoCbChannel", Shader::BUILTIN_VIDEO_CB_CHANNEL},
  {"demoloop_VideoCrChannel", Shader::BUILTIN_VIDEO_CR_CHANNEL},
};

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM> Shader::builtinNames(Shader::builtinNameEntries, sizeof(Shader::builtinNameEntries));

}
