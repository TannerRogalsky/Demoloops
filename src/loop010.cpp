#include <iostream>
#include <cmath>
#include <SDL.h>
#include "demoloop_opengl.h"
#include "hsl.h"
using namespace std;
using namespace Demoloop;

float t = 0;
const float PI = 3.1459;
const float CYCLE_LENGTH = 3;

class Loop10 : public DemoloopOpenGL {
public:
  Loop10() : DemoloopOpenGL(150, 150, 150) {
    std::cout << glGetString(GL_VERSION) << std::endl;

    Matrix4 perspective = Matrix4::perspective(PI / 4.0, (float)width / (float)height, 0.1, 100.0);
    gl.getProjection().copy(perspective);
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    const uint16_t num_vertices = 12 * 3;
    const uint16_t RADIUS = 1;
    Demoloop::Vertex vertices[num_vertices];
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
        1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };
    for (int i = 0; i < num_vertices; ++i) {
      const float t = i;
      vertices[i].x = g_vertex_buffer_data[i * 3 + 0] * RADIUS;
      vertices[i].y = g_vertex_buffer_data[i * 3 + 1] * RADIUS;
      vertices[i].z = g_vertex_buffer_data[i * 3 + 2] * RADIUS;

      auto color = hsl2rgb(t / num_vertices, 1, 0.5);

      vertices[i].r = color.r;
      vertices[i].g = color.g;
      vertices[i].b = color.b;
      vertices[i].a = 255;
    }

    gl.pushTransform();
    Matrix4& transform = gl.getTransform();
    const float cameraX = sin(cycle_ratio * PI * 2) * 4;
    const float cameraY = pow(sin(cycle_ratio * PI * 2), 2) * 3;
    const float cameraZ = cos(cycle_ratio * PI * 2) * 3;
    Matrix4 lookAt = Matrix4::lookAt({cameraX, cameraY, cameraZ}, {0, 0, 0}, {0, 1, 0});
    transform.copy(lookAt);

    gl.triangles(vertices, num_vertices);
    gl.popTransform();
  }

private:
};

int main(int, char**){
  Loop10 test;
  test.Run();

  return 0;
}
