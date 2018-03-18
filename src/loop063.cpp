#include "demoloop.h"
// #include "graphics/3d_primitives.h"
#include "graphics/shader.h"
#include "helpers.h"
#include "math_helpers.h"
#include "graphics/shader.h"
#include <array>
#include <functional>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/polyline.h"
using namespace std;
using namespace demoloop;


const float CYCLE_LENGTH = 5;

template<uint32_t N>
typename std::enable_if<N >= 3, array<Vertex, N>>::type
polygonVertices(const float &radius, const float &interval = DEMOLOOP_M_PI * 2 / N) {
  array<Vertex, N> r;
  float phi = 0.0f;
  for (uint32_t i = 0; i < N; ++i, phi += interval) {
    r[i].x = cosf(phi) * radius;
    r[i].y = sinf(phi) * radius;
    r[i].z = 1;
  }

  return r;
}

template<uint32_t N>
array<Vertex, N> getVertices(const float radius, const function<float(float)>& f) {
  array<Vertex, N> r;
  for (uint32_t i = 0; i < N; ++i) {
    const float phi = f(static_cast<float>(i) / static_cast<float>(N - 1));
    r[i].x = cosf(phi) * radius;
    r[i].y = sinf(phi) * radius;
    r[i].z = 1;
  }

  return r;
}

template<size_t N>
void lineLoop(const array<Vertex, N> &vertices, glm::mat4 transform) {
  gl.lineLoop(vertices.data(), N, transform);
}

template<size_t N>
void lineStrip(const array<Vertex, N> &vertices, glm::mat4 transform) {
  gl.lineStrip(vertices.data(), N, transform);
}

template<size_t N>
void line(MiterJoinPolyline &line, const array<Vertex, N> &vertices, float halfwidth, float pixel_size, bool draw_overdraw, const glm::mat4 &transform) {
  static array<glm::vec2, N> temp;
  for (uint32_t i = 0; i < N; ++i) {
    memcpy(&temp[i], &vertices[i % N], sizeof(glm::vec2));
  }
  line.render((float *)temp.data(), temp.size() * 2, halfwidth, pixel_size, draw_overdraw);
  line.draw(transform);
}

template<size_t N>
void line(MiterJoinPolyline &polyline, const array<Vertex, N> &vertices, float halfwidth, float pixel_size, bool draw_overdraw) {
  line(polyline, vertices, halfwidth, pixel_size, draw_overdraw, glm::mat4());
}

template<size_t N>
void lineLoop(MiterJoinPolyline &line, const array<Vertex, N> &vertices, float halfwidth, float pixel_size, bool draw_overdraw, const glm::mat4 &transform) {
  static array<glm::vec2, N + 1> temp;
  for (uint32_t i = 0; i < N + 1; ++i) {
    memcpy(&temp[i], &vertices[i % N], sizeof(glm::vec2));
  }
  line.render((float *)temp.data(), temp.size() * 2, halfwidth, pixel_size, draw_overdraw);
  line.draw(transform);
}

template<size_t N>
void lineLoop(MiterJoinPolyline &polyline, const array<Vertex, N> &vertices, float halfwidth, float pixel_size, bool draw_overdraw) {
  lineLoop(polyline, vertices, halfwidth, pixel_size, draw_overdraw, glm::mat4());
}

template<size_t N>
array<Vertex, N> spline(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
  array<Vertex, N> vertices;

  const float x1 = a.x;
  const float x2 = b.x;
  const float x3 = c.x;
  const float y1 = a.y;
  const float y2 = b.y;
  const float y3 = c.y;

  for (uint32_t i = 0; i < N; ++i) {
    const float t = static_cast<float>(i) / (N - 1);
    Vertex &v = vertices[i];
    const float x = mix(a.x, c.x, t);

    const float a1 = (x1 * (y2 - y3) +
                      x2 * (y3 - y1) +
                      x3 * (y1 - y2)) /
                      (2 * powf(x1 - x2, 2) * (x1 - x3) * (x2 - x3));
    const float a2 = ((x2 - x1) / (x2 - x3)) * a1;
    const float b1 = (powf(x1, 2) * (y3 - y2) -
                      x1 * (x2 * (-3 * y1 + y2 + 2 * y3) + 3 * x3 * (y1 - y2)) +
                      powf(x2, 2) * (y3 - y1) +
                      x2 * x3 * (y2 - y1) +
                      2 * powf(x3, 2) * (y1 - y2)) /
                      (2 * (x1 - x2) * (x1 - x3) * (x2 - x3));
    const float b2 = (2 * powf(x1, 2) * (y2 - y3) +
                      x2 * (x1 * (y3 - y2) + x3 * (2 * y1 + y2 - 3 * y3)) +
                      3 * x1 * x3 * (y3 - y2) +
                      powf(x2, 2) * (y3 - y1) +
                      powf(x3, 2) * (y2 - y1)) /
                      (2 * (x1 - x2) * (x1 - x3) * (x2 - x3));


    if (x < x2) {
      v.y = a1 * powf(x - x1, 3) + b1 * (x - x1) + y1;
    } else {
      v.y = a2 * powf(x - x3, 3) + b2 * (x - x3) + y3;
    }
    v.x = x;
    v.z = 1;
  }
  return vertices;
}

template<size_t N>
array<Vertex, N> spline(const Vertex &a, const Vertex &b, const Vertex &c) {
  glm::vec2 e, f, g;
  memcpy(&e, &a, sizeof(glm::vec2));
  memcpy(&f, &b, sizeof(glm::vec2));
  memcpy(&g, &c, sizeof(glm::vec2));
  return spline<N>(e, f, g);
}

const static std::string glowShaderCode = R"===(
#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float l = 2.0 * abs(floor(tc.x + 0.5) - tc.x);
  return vec4(color.rgb, color.a * l);
}
#endif
)===";

function<float(float)> circleInOut(const float t) {
  return [t](const float i) {
    if (t < 0.5) {
      return i * DEMOLOOP_M_PI * 2.0 * (t * 2.0);
    } else {
      const float offset = DEMOLOOP_M_PI * 2.0 * ((t - 0.5) * 2.0);
      return offset + (DEMOLOOP_M_PI * 2.0 - offset) * i;
    }
  };
}

class Loop063 : public Demoloop {
public:
  Loop063() : Demoloop(CYCLE_LENGTH, 720, 720, 0, 0, 0), glowShader({glowShaderCode, glowShaderCode}) {
    glDisable(GL_DEPTH_TEST);
    // glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    // gl.getProjection() = perspective;
    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  ~Loop063() {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    glm::mat4 transform;

    const RGB blue = {25, 135, 225};
    const RGB white = {225, 225, 255};
    const RGB beige = {250, 215, 187};

    glowLine(getVertices<100>(width * 0.4, circleInOut(cycle_ratio)), width * 0.05, transform, blue);
    glowLine(getVertices<100>(width * 0.38, circleInOut(cycle_ratio)), width * 0.05, transform, beige);
  }

private:
  Shader glowShader;
  MiterJoinPolyline noneLine;

  template<size_t N>
  void glowLine(array<Vertex, N> vertices, float glowWidth, const glm::mat4& transform, const RGB& color) {
    const static float pixel_size = 1;
    const static bool draw_overdraw = false;

    glowShader.attach();
    setColor(color, 0.25 * 255);
    line(noneLine, vertices, glowWidth, pixel_size, draw_overdraw, transform);
    glowShader.detach();

    setColor(color);
    line(noneLine, vertices, 1, pixel_size, draw_overdraw, transform);
  }
};

int main(int, char**){
  Loop063 test;
  test.Run();

  return 0;
}
