#include <iostream>
#include "demoloop.h"
#include "graphics/2d_primitives.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

const int num_circle_verts = 10;
const int num_vertices = 5;
const int dot_count = 20;

class Loop5 : public Demoloop {
public:
  Loop5() : Demoloop(150, 150, 150) {
    glDisable(GL_DEPTH_TEST);
  }

  void Update(float dt) {
    t += dt;

    const float RADIUS = height / 3;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    int ox = width / 2, oy = height / 2;

    const float interval = (DEMOLOOP_M_PI * 2) / num_vertices;
    float xCoords[num_vertices];
    float yCoords[num_vertices];
    for (int i = 0; i < num_vertices; ++i) {
      float t = i;
      xCoords[i] = cos(interval * t - DEMOLOOP_M_PI / 10) * RADIUS + ox;
      yCoords[i] = sin(interval * t - DEMOLOOP_M_PI / 10) * RADIUS + oy;
    }

    auto color = hsl2rgb(cycle_ratio, 1, 0.5);
    setColor(color);
    polygon(gl, xCoords, yCoords, num_vertices);

    int count = 0;

    const RGB black = {0, 0, 0};
    setColor(black);
    for (int v = 0; v < num_vertices; ++v) {
      const float angularOffset = interval * v;
      for (int t = 0; t < dot_count; ++t) {
        const float interval_cycle_ratio = fmod(static_cast<float>(t) / dot_count + cycle_ratio, 1);

        const float x1 = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 2 - DEMOLOOP_M_PI / 2 + angularOffset) * interval_cycle_ratio * RADIUS;
        const float y1 = sin(interval_cycle_ratio * DEMOLOOP_M_PI * 2 - DEMOLOOP_M_PI / 2 + angularOffset) * interval_cycle_ratio * RADIUS;

        glm::mat4 m;
        m = glm::translate(m, {x1 + ox, y1 + oy, 0});
        for (uint32_t i = 0; i < num_circle_verts - 1; ++i) {
          circleVerts[count].x = cosf(0) * 3;
          circleVerts[count].y = sinf(0) * 3;
          circleVerts[count].z = 1;
          circleVerts[count].a = 255 * interval_cycle_ratio;
          applyColor(circleVerts[count], black);
          applyMatrix(circleVerts[count], m);
          count++;

          circleVerts[count].x = cosf(interval * i) * 3;
          circleVerts[count].y = sinf(interval * i) * 3;
          circleVerts[count].z = 1;
          circleVerts[count].a = 255 * interval_cycle_ratio;
          applyColor(circleVerts[count], black);
          applyMatrix(circleVerts[count], m);
          count++;

          circleVerts[count].x = cosf(interval * (i + 1)) * 3;
          circleVerts[count].y = sinf(interval * (i + 1)) * 3;
          circleVerts[count].z = 1;
          circleVerts[count].a = 255 * interval_cycle_ratio;
          applyColor(circleVerts[count], black);
          applyMatrix(circleVerts[count], m);
          count++;
        }


        if (t == 0) {
          const int n = (v + 1) % num_vertices;
          const float x2 = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 2 - DEMOLOOP_M_PI / 2 + angularOffset) * interval_cycle_ratio * RADIUS;
          const float y2 = sin(interval_cycle_ratio * DEMOLOOP_M_PI * 2 - DEMOLOOP_M_PI / 2 + angularOffset) * interval_cycle_ratio * RADIUS;
          const float x3 = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 2 - DEMOLOOP_M_PI / 2 + (interval * n)) * interval_cycle_ratio * RADIUS;
          const float y3 = sin(interval_cycle_ratio * DEMOLOOP_M_PI * 2 - DEMOLOOP_M_PI / 2 + (interval * n)) * interval_cycle_ratio * RADIUS;

          line(gl, x2 + ox, y2 + oy, xCoords[v], yCoords[v]);
          line(gl, x2 + ox, y2 + oy, x3 + ox, y3 + oy);
        }
      }
    }

    for (float i = 0; i < dot_count; ++i) {
      const float interval_cycle_ratio = fmod(i / dot_count + cycle_ratio, 1);

      const float x1 = cos(interval_cycle_ratio * DEMOLOOP_M_PI * 2 - DEMOLOOP_M_PI / 2) * RADIUS;
      const float y1 = sin(interval_cycle_ratio * DEMOLOOP_M_PI * 2 - DEMOLOOP_M_PI / 2) * RADIUS;

      glm::mat4 m;
      m = glm::translate(m, {x1 + ox, y1 + oy, 0});
      for (uint32_t i = 0; i < num_circle_verts - 1; ++i) {
        circleVerts[count].x = cosf(0) * 3;
        circleVerts[count].y = sinf(0) * 3;
        circleVerts[count].z = 1;
        applyColor(circleVerts[count], black);
        applyMatrix(circleVerts[count], m);
        count++;

        circleVerts[count].x = cosf(interval * i) * 3;
        circleVerts[count].y = sinf(interval * i) * 3;
        circleVerts[count].z = 1;
        applyColor(circleVerts[count], black);
        applyMatrix(circleVerts[count], m);
        count++;

        circleVerts[count].x = cosf(interval * (i + 1)) * 3;
        circleVerts[count].y = sinf(interval * (i + 1)) * 3;
        circleVerts[count].z = 1;
        applyColor(circleVerts[count], black);
        applyMatrix(circleVerts[count], m);
        count++;
      }
    }

    gl.triangles(circleVerts, count);
  }

private:
  Vertex circleVerts[(num_circle_verts - 1) * 3 * dot_count * (num_vertices + 1)];
};

int main(int, char**){
  Loop5 loop;
  loop.Run();

  return 0;
}
