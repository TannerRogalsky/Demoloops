
#include <algorithm>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/2d_primitives.h"
#include "graphics/mesh.h"
#include "graphics/canvas.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;
const uint32_t arms = 5;
const uint32_t trisPerArm = 20;
const uint32_t numTris = arms * trisPerArm;

const float RADIUS = 10;
const float interval = DEMOLOOP_M_PI * 2 / 3;
const Triangle defaultTriangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 1},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 1},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 1}
};


class Loop033 : public Demoloop {
public:
  Loop033() : Demoloop(CYCLE_LENGTH, 150, 150, 150), mesh(cube(0, 0, 0, 1)), canvas(height, height)  {
    glEnable(GL_CULL_FACE);

    mesh.setTexture(&canvas);
    mesh.mIndices[6] = 0+4;
    mesh.mIndices[7] = 1+4;
    mesh.mIndices[8] = 2+4;
    mesh.mIndices[9] = 0+4;
    mesh.mIndices[10] = 2+4;
    mesh.mIndices[11] = 3+4;
    mesh.buffer();
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();
    float tau_cycle = cycle_ratio * DEMOLOOP_M_PI * 2;

    {
      setCanvas(&canvas);

      glClearColor(0.0f, 0.0f, 0.0f, 0.1f);
      glClear(GL_COLOR_BUFFER_BIT);

      setBlendMode(BLEND_ALPHA, BLENDALPHA_PREMULTIPLIED);

      GL::TempTransform t1(gl);
      t1.get() = glm::translate(t1.get(), {canvas.getWidth() / 2, canvas.getHeight() / 2, 0});
      glm::vec3 twoDAxis = {0, 0 , 1};

      const uint32_t width = canvas.getWidth() * 1.3;

      for (uint32_t i = 0; i < numTris; ++i) {
        const float t = i;
        const float armIndex = i % arms;
        const float i_cycle_ratio = fmod(t / numTris + cycle_ratio, 1);

        float x = cosf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * width / 4;
        x *= sinf(cycle_ratio * DEMOLOOP_M_PI * 2);
        x += pow(sinf(i_cycle_ratio * DEMOLOOP_M_PI), 2) * width / 10;
        float y = sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * width / 4;
        y *= cosf(cycle_ratio * DEMOLOOP_M_PI * 2);
        float d = sqrt(x * x + y * y);

        glm::mat4 m;
        m = glm::rotate(m, (float)(-DEMOLOOP_M_PI) / 2, twoDAxis);
        m = glm::rotate(m, (float)DEMOLOOP_M_PI * 2 / arms * armIndex, twoDAxis);
        m = glm::translate(m, glm::vec3(x, y, 1));

        triangles[i] = defaultTriangle;
        applyMatrix(triangles[i], m);
        applyColor(triangles[i], hsl2rgb(fmod(cycle_ratio + d / (width / 2) * 0.65, 1), 1, 0.5));
        triangles[i].a.a = 150;
        triangles[i].b.a = 150;
        triangles[i].c.a = 150;
      }

      gl.bindTexture(gl.getDefaultTexture());
      gl.triangles(triangles, numTris);

      setCanvas();
    }

    const glm::vec3 eye = glm::rotate(glm::vec3(4, 3, 1), tau_cycle, glm::vec3(0, 1, 0));
    const glm::vec3 target = {0, 0, 0};
    const glm::vec3 up = {0, 1, 0};
    glm::mat4 camera = glm::lookAt(eye, target, up);

    GL::TempTransform t1(gl);
    t1.get() = camera;

    GL::TempProjection p1(gl);
    p1.get() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

    setBlendMode(BLEND_ALPHA, BLENDALPHA_MULTIPLY);
    mesh.draw();
  }
private:
  Mesh mesh;
  Canvas canvas;
  Triangle triangles[numTris];
};

int main(int, char**){
  Loop033 loop;
  loop.Run();

  return 0;
}
