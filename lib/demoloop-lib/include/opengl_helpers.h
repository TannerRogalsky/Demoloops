#pragma once

#include <GL/glew.h>
#include <string>

std::string createVertexCode(const std::string &vertexShaderSource);
std::string createFragmentCode(const std::string &fragmentShaderSource);
GLuint loadProgram(const std::string &vertexShaderSource, const std::string &fragmentShaderSource);
void printProgramLog(GLuint program);
void printShaderLog(GLuint shader);
