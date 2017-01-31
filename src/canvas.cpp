
#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/canvas.h"
using namespace std;
using namespace demoloop;

class Geometric : public Demoloop {
public:
  Geometric() : Demoloop(0, 0, 0), canvas(width, height) {
    glDisable(GL_DEPTH_TEST);
  }

  void Update(float) {
    const RGB pink(255, 132, 242);

    // test case
    setCanvas(&canvas);
      setColor(pink);
      rectangle(gl, 50, 50, 50, 50);
    setCanvas();

    setColor(255, 255, 255);
    canvas.draw();


    // reference
    setColor(pink);
    rectangle(gl, 100, 50, 50, 50);
  }

private:
  Canvas canvas;
};

int main(int, char**){
  Geometric test;
  test.Run();

  return 0;
}
