#include "opengl_helpers.h"
#include <SDL.h>
#include <sstream>
#include <iostream>

const char* SYNTAX =  "#ifndef GL_ES\n"
                      "#define lowp\n"
                      "#define mediump\n"
                      "#define highp\n"
                      "#endif\n"
                      "#define number float\n"
                      "#define Image sampler2D\n"
                      "#define extern uniform\n"
                      "#define Texel texture2D\n"
                      "#pragma optionNV(strict on)\n";

const char* VERTEX_HEADER = "#define VERTEX\n"
                            "#define LOVE_PRECISE_GAMMA\n"
                            "\n"
                            "attribute vec4 VertexPosition;\n"
                            "attribute vec4 VertexTexCoord;\n"
                            "attribute vec4 VertexColor;\n"
                            "attribute vec4 ConstantColor;\n"
                            "\n"
                            "varying vec4 VaryingTexCoord;\n"
                            "varying vec4 VaryingColor;\n"
                            "\n"
                            "#ifdef GL_ES\n"
                            "uniform mediump float demoloop_PointSize;\n"
                            "#endif\n";

const char* VERTEX_FOOTER = "void main() {\n"
                            "VaryingTexCoord = VertexTexCoord;\n"
                            "VaryingColor = VertexColor * ConstantColor;\n"
                            // "VaryingColor = gammaCorrectColor(VertexColor) * ConstantColor;\n"
                            "#ifdef GL_ES\n"
                            "gl_PointSize = demoloop_PointSize;\n"
                            "#endif\n"
                            "gl_Position = position(TransformProjectionMatrix, ModelMatrix, VertexPosition);\n"
                            "}\n";

const char* FRAG_HEADER = "#define PIXEL\n"
                          "\n"
                          "#ifdef GL_ES\n"
                          "precision mediump float;\n"
                          "#endif\n"
                          "\n"
                          "varying mediump vec4 VaryingTexCoord;\n"
                          "varying mediump vec4 VaryingColor;\n"
                          "\n"
                          "#define love_Canvases gl_FragData\n"
                          "\n"
                          "uniform sampler2D _tex0_;\n";

const char* FRAG_FOOTER = "void main() {\n"
                          "// fix crashing issue in OSX when _tex0_ is unused within effect()\n"
                          "float dummy = Texel(_tex0_, vec2(.5)).r;\n"
                          "\n"
                          "// See Shader::checkSetScreenParams in Shader.cpp.\n"
                          "vec2 pixelcoord = vec2(gl_FragCoord.x, (gl_FragCoord.y * demoloop_ScreenSize.z) + demoloop_ScreenSize.w);\n"
                          "\n"
                          "gl_FragColor = effect(VaryingColor, _tex0_, VaryingTexCoord.st, pixelcoord);\n"
                          "}\n";

const char* UNIFORMS =  "\n"
                        "  // According to the GLSL ES 1.0 spec, uniform precision must match between stages,\n"
                        "  // but we can't guarantee that highp is always supported in fragment shaders...\n"
                        "  // We *really* don't want to use mediump for these in vertex shaders though.\n"
                        "  #if defined(VERTEX) || defined(GL_FRAGMENT_PRECISION_HIGH)\n"
                        "    #define DEMOLOOP_UNIFORM_PRECISION highp\n"
                        "  #else\n"
                        "    #define DEMOLOOP_UNIFORM_PRECISION mediump\n"
                        "  #endif\n"
                        "  uniform DEMOLOOP_UNIFORM_PRECISION mat4 TransformMatrix;\n"
                        "  uniform DEMOLOOP_UNIFORM_PRECISION mat4 ProjectionMatrix;\n"
                        "  uniform DEMOLOOP_UNIFORM_PRECISION mat4 TransformProjectionMatrix;\n"
                        "  uniform DEMOLOOP_UNIFORM_PRECISION mat4 ModelMatrix;\n"
                        "  uniform DEMOLOOP_UNIFORM_PRECISION mat3 NormalMatrix;\n"
                        "uniform mediump vec4 demoloop_ScreenSize;\n";

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

      std::cerr << infoLog << std::endl;
    }

    delete[] infoLog;
  } else {
    std::cerr << "Name " << program << " is not a program." << std::endl;
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
      std::cerr << infoLog << std::endl;
    }

    delete[] infoLog;
  }
  else {
    std::cerr << "Name " << shader << " is not a shader." << std::endl;
  }
}
