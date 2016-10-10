#pragma once

#include <GL/glew.h>
#include <string>

std::string createVertexCode(const char *vertexShaderSource);
std::string createFragmentCode(const char *fragmentShaderSource);
GLuint loadProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
void printProgramLog(GLuint program);
void printShaderLog(GLuint shader);
