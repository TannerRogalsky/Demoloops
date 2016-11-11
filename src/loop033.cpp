#include <iostream>
#include <algorithm>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/2d_primitives.h"
#include "graphics/mesh.h"
#include "graphics/canvas.h"
using namespace std;
using namespace demoloop;

#define MAX_VERTS 15
#define MIN_VERTS 3
#define RADIUS 1
float t = 0;
const float CYCLE_LENGTH = 10;

class Loop033 : public Demoloop {
public:
  Loop033() : Demoloop(150, 150, 150), mesh(nullptr), canvas(width, height)  {
    // gl.getProjection() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    glDisable(GL_DEPTH_TEST);

    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT_AND_BACK);
    mesh = cube(0, 0, 0, RADIUS);
    mesh->setTexture(&canvas);
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;
    float tau_cycle = cycle_ratio * DEMOLOOP_M_PI * 2;

    {
      setCanvas(&canvas);
      // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      // glClear(GL_COLOR_BUFFER_BIT);


      setBlendMode(BLEND_ALPHA, BLENDALPHA_PREMULTIPLIED);
      // setColor(255, 255, 255, 1);
      // rectangle(gl, 0, 0, width, height);

      GL::TempTransform t1(gl);
      t1.get() = glm::translate(t1.get(), {canvas.getWidth() / 2, canvas.getHeight() / 2, 0});

      const float x = cosf(tau_cycle) * canvas.getWidth() / 4;
      const float y = sinf(tau_cycle) * canvas.getHeight() / 4;
      setColor(hsl2rgb(cycle_ratio, 1, 0.5), 100);
      circle(gl, x, y, 20, 20);
      setCanvas();
    }

    // const glm::vec3 eye = glm::rotate(glm::vec3(4, 3, 10), tau_cycle, glm::vec3(0, 1, 0));
    // const glm::vec3 target = {0, 0, 0};
    // const glm::vec3 up = {0, 1, 0};
    // glm::mat4 camera = glm::lookAt(eye, target, up);

    // GL::TempTransform t1(gl);
    // t1.get() = camera;

    // setColor(255, 255, 255);
    // mesh->draw();

    setBlendMode(BLEND_ALPHA, BLENDALPHA_MULTIPLY);
    setColor(255, 255, 255);
    canvas.draw(glm::mat4());
  }
private:
  Mesh *mesh;
  Canvas canvas;
};

int main(int, char**){
  Loop033 loop;
  loop.Run();

  return 0;
}
