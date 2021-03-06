#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "hsl.h"
using namespace std;
using namespace demoloop;

#define NUM_VERTS 60

const uint32_t CYCLE_LENGTH = 6 * 3;

class Loop016 : public Demoloop {
public:
  Loop016() : Demoloop(CYCLE_LENGTH, 150, 150, 150), RADIUS(height / 10) {
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

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
    gl.getTransform() = glm::rotate(gl.getTransform(), (float)DEMOLOOP_M_PI / 6, {0, 0, 1});
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    setColor(255, 255, 255);
    gl.lines(vertices, NUM_VERTS);

    const float interval = DEMOLOOP_M_PI * 2 / 6;

    for (int i = 0; i < 6; ++i) {
      gl.pushTransform();
      glm::mat4 &m = gl.getTransform();
      m = glm::rotate(m, interval * i - cycle_ratio * (float)DEMOLOOP_M_PI * 2, {0, 0, 1});
      m = glm::translate(m, {RADIUS * 2, 0, 0});

      gl.lines(vertices, NUM_VERTS);

      gl.popTransform();
    }

    for (int i = 0; i < 6; ++i) {
      gl.pushTransform();
      glm::mat4 &m = gl.getTransform();
      m = glm::rotate(m, interval * i + cycle_ratio * (float)DEMOLOOP_M_PI * 2, {0, 0, 1});
      m = glm::translate(m, {RADIUS * 4, 0, 0});

      gl.lines(vertices, NUM_VERTS);

      gl.popTransform();
    }

    const static int numOutsideLines = 3 * 4 * 2;
    Vertex outsideLines[numOutsideLines];
    int index = 0;
    for (int i = 0; i < 6; i+=2) {
      const float x1 = RADIUS * 4 * cosf(i * interval + cycle_ratio * DEMOLOOP_M_PI * 2);
      const float y1 = RADIUS * 4 * sinf(i * interval + cycle_ratio * DEMOLOOP_M_PI * 2);

      outsideLines[index].x = x1;
      outsideLines[index].y = y1;
      outsideLines[index].z = 0;
      index++;
      // outsideLines[index].x = RADIUS * 4 * cosf((i + 1) * interval + cycle_ratio * DEMOLOOP_M_PI * 2);
      // outsideLines[index].y = RADIUS * 4 * sinf((i + 1) * interval + cycle_ratio * DEMOLOOP_M_PI * 2);
      // outsideLines[index].z = 0;
      // index++;
      outsideLines[index].x = 0;
      outsideLines[index].y = 0;
      outsideLines[index].z = 0;
      index++;

      for (int j = 0; j < 6; j+=2) {
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

    const static int numInsideLines = 3 * 3 * 2;
    Vertex insideLines[numInsideLines];
    index = 0;
    for (int i = 1; i < 6; i += 2) {
      const float x1 = RADIUS * 2 * cosf(i * interval - cycle_ratio * DEMOLOOP_M_PI * 2);
      const float y1 = RADIUS * 2 * sinf(i * interval - cycle_ratio * DEMOLOOP_M_PI * 2);

      for (int j = 1; j < 6; j += 2) {
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
  Vertex vertices[NUM_VERTS];
};

int main(int, char**){
  Loop016 loop;
  loop.Run();

  return 0;
}
