#include <iostream>
#include <cmath>
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include "demoloop.h"
#include "helpers.h"
using namespace std;

class Test3 : public Demoloop {
public:
  Test3() : Demoloop(150, 150, 150) {
  }

  ~Test3() {
  }

  void Update(float dt) {
    lineRGBA(renderer, 0, 0, 100, 100, 255, 255, 0, 255);
    filledCircleColor(renderer, 100, 100, 50, 0xFF0000FF);
  }

private:
};

int main(int, char**){
  Test3 test;
  test.Run();

  return 0;
}
