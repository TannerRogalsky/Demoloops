#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/2d_primitives.h"
#include <iostream>
using namespace std;
using namespace demoloop;


const uint32_t CYCLE_LENGTH = 1;
const uint32_t NUM_CYCLES = 10;

float jmap(const float &t, const float &s0, const float &s1, const float &e0, const float &e1) {
  float v = 0;
  if (s0 != s1) v = (t-s0)/(s1-s0);
  v = e0 + (v*(e1-e0));
  return v;
}

float random(float min, float max) {
  return jmap(rand(), 0, RAND_MAX, min, max);
}

float blob(float t) {
  float e2p5 = 0.00193045f;  // e^-(2.5^2), which is < .004 (about 1/255)
  float ev = jmap(t, 0, 1, 0, 2.5f);
  return jmap((float)exp(-(ev*ev)), 1, e2p5, 1, 0);;
}

float getHeight(float s, const float *center, const float *radius, const float *amplitude, const uint32_t &num_humps)  {
  float hgt = 0;
  for (uint32_t i=0; i<num_humps; i++) {
    float d = abs(s-center[i])/radius[i];
    if (d < 1) {
      // float h = cosf(jmap(d, 0, 1, 0, DEMOLOOP_M_PI / 2));
      float h = blob(d);
      hgt += amplitude[i] * h;
    }
  }
  return hgt;
};

template <
  typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
> constexpr T mix(T const& a, T const& b, const float& ratio) {
  return a * (1.0f - ratio) + b * ratio;
}

float cubicEaseInOut(float t,float b , float c, float d) {
  t/=d/2;
  if (t < 1) return c/2*t*t*t + b;
  t-=2;
  return c/2*(t*t*t + 2) + b;
}

class WaveVisualization : public Demoloop {
public:
  WaveVisualization() : Demoloop(CYCLE_LENGTH, 150, 150, 150) {
  }

  void Update() {
    const float SCALE = 1;

    GL::TempTransform t1(gl);
    glm::mat4 &m = t1.get();
    m = glm::translate(m, {0, height, 0});
    m = glm::scale(m, {SCALE, SCALE, 1});

    const uint32_t num_humps = 7;
    float center[num_humps];
    float radius[num_humps];
    float amplitude[num_humps];

    srand(fmod(floor(getTime() / CYCLE_LENGTH), NUM_CYCLES));
    // srand(0);

    for (uint32_t hump_index = 0; hump_index < num_humps; ++hump_index) {
      center[hump_index] = random(0, 1);
      radius[hump_index] = random(.05, .4);
      amplitude[hump_index] = random(.5, 2) * height / 4;
    }

    for (int i = 0; i < width; ++i) {
      float x = i;
      float t = x / width;
      float y1 = -getHeight(t, center, radius, amplitude, num_humps);
      float y2 = 0;
      setColor(hsl2rgb((float)i / width, 1, 0.5));
      line(gl, x, y1, x + 1, y2);
    }
  }

private:
};

int main(int, char**){
  WaveVisualization test;
  test.Run();

  return 0;
}
