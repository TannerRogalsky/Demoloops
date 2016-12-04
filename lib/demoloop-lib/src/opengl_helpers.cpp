#include "opengl_helpers.h"
#include <SDL.h>
#include <sstream>

const char* SYNTAX = R"===(
#ifndef GL_ES
#define lowp
#define mediump
#define highp
#endif
#define number float
#define Image sampler2D
#define extern uniform
#define Texel texture2D
// #pragma optionNV(strict on)
)===";

const char* VERTEX_HEADER = R"===(
#define VERTEX

attribute vec4 VertexPosition;
attribute vec4 VertexTexCoord;
attribute vec4 VertexColor;
attribute vec4 ConstantColor;

varying vec4 VaryingTexCoord;
varying vec4 VaryingColor;

#ifdef GL_ES
uniform mediump float demoloop_PointSize;
#endif
)===";

const char* VERTEX_FOOTER = R"===(
void main() {
  VaryingTexCoord = VertexTexCoord;
  VaryingColor = VertexColor * ConstantColor;

  #ifdef GL_ES
  gl_PointSize = demoloop_PointSize;
  #endif
  gl_Position = position(TransformProjectionMatrix, ModelMatrix, VertexPosition);
}
)===";

const char* FRAG_HEADER = R"===(
#define PIXEL

#ifdef GL_ES
precision mediump float;
#endif

varying mediump vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

#define demoloop_Canvases gl_FragData

uniform sampler2D _tex0_;
)===";

const char* FRAG_FOOTER = R"===(
void main() {
  // fix crashing issue in OSX when _tex0_ is unused within effect()
  float dummy = Texel(_tex0_, vec2(.5)).r;

  // See Shader::checkSetScreenParams in Shader.cpp.
  vec2 pixelcoord = vec2(gl_FragCoord.x, (gl_FragCoord.y * demoloop_ScreenSize.z) + demoloop_ScreenSize.w);

  gl_FragColor = effect(VaryingColor, _tex0_, VaryingTexCoord.st, pixelcoord);
}
)===";

const char* UNIFORMS = R"===(
// According to the GLSL ES 1.0 spec, uniform precision must match between stages,
// but we can't guarantee that highp is always supported in fragment shaders...
// We *really* don't want to use mediump for these in vertex shaders though.
#if defined(VERTEX) || defined(GL_FRAGMENT_PRECISION_HIGH)
  #define DEMOLOOP_UNIFORM_PRECISION highp
#else
  #define DEMOLOOP_UNIFORM_PRECISION mediump
#endif
uniform DEMOLOOP_UNIFORM_PRECISION mat4 TransformMatrix;
uniform DEMOLOOP_UNIFORM_PRECISION mat4 ProjectionMatrix;
uniform DEMOLOOP_UNIFORM_PRECISION mat4 TransformProjectionMatrix;
uniform DEMOLOOP_UNIFORM_PRECISION mat4 ModelMatrix;
uniform DEMOLOOP_UNIFORM_PRECISION mat3 NormalMatrix;
uniform mediump vec4 demoloop_ScreenSize;
)===";

const char* getVersionPragma() {
  int profileMask;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profileMask);
  return profileMask == SDL_GL_CONTEXT_PROFILE_ES ? "#version 100\n" : "#version 120\n";
}

const char* getLinePragma() {
  int profileMask;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profileMask);
  return profileMask == SDL_GL_CONTEXT_PROFILE_ES ? "#line 1\n" : "#line 0\n";
}

std::string createVertexCode(const std::string &vertexShaderSource) {
  std::stringstream ss;
  ss << getVersionPragma() << SYNTAX << VERTEX_HEADER << UNIFORMS << getLinePragma() << vertexShaderSource << "\n" << VERTEX_FOOTER;
  return ss.str();
}

std::string createFragmentCode(const std::string &fragmentShaderSource) {
  std::stringstream ss;
  ss << getVersionPragma() << SYNTAX << FRAG_HEADER << UNIFORMS << getLinePragma() << fragmentShaderSource << "\n" << FRAG_FOOTER;
  return ss.str();
}

void printProgramLog(GLuint program) {
  if(glIsProgram(program)) {
    int infoLogLength = 0;
    int maxLength = infoLogLength;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    char* infoLog = new char[ maxLength ];

    glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
    if(infoLogLength > 0) {
      printf("%s\n", infoLog);
    }

    delete[] infoLog;
  } else {
    printf("Name %d is not a program\n", program);
  }
}

void printShaderLog(GLuint shader) {
  if(glIsShader(shader)) {
    int infoLogLength = 0;
    int maxLength = infoLogLength;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    char* infoLog = new char[ maxLength ];

    glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
    if(infoLogLength > 0) {
      printf( "%s\n", infoLog );
    }

    delete[] infoLog;
  }
  else {
    printf("Name %d is not a shader\n", shader);
  }
}
