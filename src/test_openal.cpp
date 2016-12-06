#include "demoloop.h"
#include "audio/source.h"

using namespace std;
using namespace demoloop;

class Test : public Demoloop {
public:
  Test() : Demoloop(150, 150, 150), source("test_openal/short.ogg") {
    source.play();
  }

  ~Test() {
  }

  void Update(float) {
    source.update();
  }

private:
  Source source;
};

int main(int, char**){
  Test test;
  test.Run();

  return 0;
}
