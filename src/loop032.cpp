
#include <algorithm>
#include "demoloop.h"
using namespace std;
using namespace demoloop;

#define MAX_VERTS 15
#define MIN_VERTS 3
#define RADIUS 20

const uint32_t nw = 640 / (RADIUS * 2);
const uint32_t nh = 480 / (RADIUS * 2);
const uint32_t num = nw * nh;

float t = 0;
const float CYCLE_LENGTH = 10;

class Loop032 : public Demoloop {
public:
  Loop032() : Demoloop(150, 150, 150) {
    // Matrix4 &m = gl.getTransform();
    // m.translate(width / 2, height / 2);
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    uint32_t count = 0;

    for (uint32_t ix = 0; ix < nw; ++ix) {
      for (uint32_t iy = 0; iy < nh; ++iy) {
        const uint32_t i = ix + (iy * nh);

        const float t = i;
        const float i_cycle_ratio = fmod(t / num + cycle_ratio, 1);
        const float vertex_cycle = powf(sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2), 2);
        const uint32_t num_vertices = min((int)(vertex_cycle * (MAX_VERTS - MIN_VERTS) + MIN_VERTS + 1), MAX_VERTS);
        const float i_num_vertices = vertex_cycle * (MAX_VERTS - MIN_VERTS) + MIN_VERTS;
        const float interval = (DEMOLOOP_M_PI * 2) / i_num_vertices;

        float x = ix * RADIUS * 2 + RADIUS;
        float y = iy * RADIUS * 2 + RADIUS;

        glm::mat4 m;
        m = glm::translate(m, {x, y, 1});

        const auto color = hsl2rgb(i_cycle_ratio, 1, 0.5);

        for (uint32_t i = 0; i < num_vertices - 1; ++i) {
          float t = i;

          vertices[count].x = cosf(0) * RADIUS;
          vertices[count].y = sinf(0) * RADIUS;
          vertices[count].z = 1;
          applyMatrix(vertices[count], m);
          applyColor(vertices[count], color);
          count++;

          vertices[count].x = cosf(interval * t) * RADIUS;
          vertices[count].y = sinf(interval * t) * RADIUS;
          vertices[count].z = 1;
          applyMatrix(vertices[count], m);
          applyColor(vertices[count], color);
          count++;

          vertices[count].x = cosf(interval * (t + 1)) * RADIUS;
          vertices[count].y = sinf(interval * (t + 1)) * RADIUS;
          vertices[count].z = 1;
          applyMatrix(vertices[count], m);
          applyColor(vertices[count], color);
          count++;
        }
      }
    }

    gl.triangles(vertices, count);
  }
private:
  Vertex vertices[(MAX_VERTS - 2) * 3 * num];
};

int main(int, char**){
  Loop032 loop;
  loop.Run();

  return 0;
}
