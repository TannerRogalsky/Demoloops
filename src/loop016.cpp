#include <iostream>
#include <algorithm>
#include "demoloop_opengl.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;

#define NUM_VERTS 60

float t = 0;
const float CYCLE_LENGTH = 6 * 3;

class Loop016 : public Demoloop::DemoloopOpenGL {
public:
  Loop016() : Demoloop::DemoloopOpenGL(150, 150, 150), RADIUS(height / 10) {
    glDisable(GL_DEPTH_TEST);

    float phi = 0.0f;
    const float interval = DEMOLOOP_M_PI * 2 / NUM_VERTS * 2;

    for (int i = 0; i < NUM_VERTS - 1; i+=2, phi += interval) {
      vertices[i].x = RADIUS * cosf(phi);
      vertices[i].y = RADIUS * sinf(phi);
      vertices[i].z = 1;

      vertices[i + 1].x = RADIUS * cosf(phi + interval);
      vertices[i + 1].y = RADIUS * sinf(phi + interval);
      vertices[i + 1].z = 1;
    }

    Demoloop::Matrix4 &m = gl.getTransform();
    m.translate(width / 2, height / 2);
    m.rotate(DEMOLOOP_M_PI / 6);
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    setColor(255, 255, 255);
    gl.lines(vertices, NUM_VERTS);

    const float interval = DEMOLOOP_M_PI * 2 / 6;

    for (int i = 0; i < 6; ++i) {
      gl.pushTransform();
      Demoloop::Matrix4 &m = gl.getTransform();
      m.rotate(interval * i - cycle_ratio * DEMOLOOP_M_PI * 2);
      m.translate(RADIUS * 2, 0);

      gl.lines(vertices, NUM_VERTS);

      gl.popTransform();
    }

    for (int i = 0; i < 6; ++i) {
      gl.pushTransform();
      Demoloop::Matrix4 &m = gl.getTransform();
      m.rotate(interval * i + cycle_ratio * DEMOLOOP_M_PI * 2);
      m.translate(RADIUS * 4, 0);

      gl.lines(vertices, NUM_VERTS);

      gl.popTransform();
    }

    const static int numOutsideLines = 6 * 7 * 2;
    Demoloop::Vertex outsideLines[numOutsideLines];
    int index = 0;
    for (int i = 0; i < 6; ++i) {
      const float x1 = RADIUS * 4 * cosf(i * interval + cycle_ratio * DEMOLOOP_M_PI * 2);
      const float y1 = RADIUS * 4 * sinf(i * interval + cycle_ratio * DEMOLOOP_M_PI * 2);

      outsideLines[index].x = x1;
      outsideLines[index].y = y1;
      outsideLines[index].z = 0;
      index++;
      outsideLines[index].x = RADIUS * 4 * cosf((i + 1) * interval + cycle_ratio * DEMOLOOP_M_PI * 2);
      outsideLines[index].y = RADIUS * 4 * sinf((i + 1) * interval + cycle_ratio * DEMOLOOP_M_PI * 2);
      outsideLines[index].z = 0;
      index++;

      for (int j = 0; j < 6; ++j) {
        // const int n = (i + j) % 6;
        const float x2 = RADIUS * 2 * cosf(j * interval - cycle_ratio * DEMOLOOP_M_PI * 2);
        const float y2 = RADIUS * 2 * sinf(j * interval - cycle_ratio * DEMOLOOP_M_PI * 2);

        outsideLines[index].x = x1;
        outsideLines[index].y = y1;
        outsideLines[index].z = 0;
        index++;
        outsideLines[index].x = x2;
        outsideLines[index].y = y2;
        outsideLines[index].z = 0;
        index++;
      }
    }
    setColor(0, 0, 0);
    gl.lines(outsideLines, numOutsideLines);

    const static int numInsideLines = 6 * 6 * 2;
    Demoloop::Vertex insideLines[numInsideLines];
    index = 0;
    for (int i = 0; i < 6; ++i) {
      const float x1 = RADIUS * 2 * cosf(i * interval - cycle_ratio * DEMOLOOP_M_PI * 2);
      const float y1 = RADIUS * 2 * sinf(i * interval - cycle_ratio * DEMOLOOP_M_PI * 2);

      for (int j = 0; j < 6; ++j) {
        const float x2 = RADIUS * 2 * cosf(j * interval - cycle_ratio * DEMOLOOP_M_PI * 2);
        const float y2 = RADIUS * 2 * sinf(j * interval - cycle_ratio * DEMOLOOP_M_PI * 2);

        insideLines[index].x = x1;
        insideLines[index].y = y1;
        insideLines[index].z = 0;
        index++;
        insideLines[index].x = x2;
        insideLines[index].y = y2;
        insideLines[index].z = 0;
        index++;
      }
    }
    // setColor(255, 0, 0);
    gl.lines(insideLines, numInsideLines);
  }
private:
  const float RADIUS;
  Demoloop::Vertex vertices[NUM_VERTS];
};

int main(int, char**){
  Loop016 loop;
  loop.Run();

  return 0;
}
