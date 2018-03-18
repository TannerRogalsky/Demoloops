#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/shader.h"
#include "helpers.h"
#include "math_helpers.h"
#include "graphics/shader.h"
#include <array>
#include <algorithm>
#include <glm/gtx/rotate_vector.hpp>
#include "poly2tri.h"
using namespace std;
using namespace demoloop;


const float CYCLE_LENGTH = 5;

template<uint32_t N>
array<Vertex, N * 4> rectangle(const float &width, const float &height) {
  array<Vertex, N * 4> r;

  const float halfWidth = width / 2, halfHeight = height / 2;

  uint32_t index = 0;
  for (uint32_t i = 0; i < N; ++i) {
    r[index].x = -halfWidth + halfWidth * 2 * static_cast<float>(i) / N;
    r[index].y = -halfHeight;
    index++;
  }
  for (uint32_t i = 0; i < N; ++i) {
    r[index].x = halfWidth;
    r[index].y = -halfHeight + halfHeight * 2 * static_cast<float>(i) / N;
    index++;
  }
  for (uint32_t i = 0; i < N; ++i) {
    r[index].x = halfWidth - (halfWidth * 2) * static_cast<float>(i) / N;
    r[index].y = halfHeight;
    index++;
  }
  for (uint32_t i = 0; i < N; ++i) {
    r[index].x = -halfWidth;
    r[index].y = halfHeight - halfHeight * 2 * static_cast<float>(i) / N;
    index++;
  }

  return r;
}

vector<Vertex> triangulate(const size_t num, const Vertex *vertices) {
  const static auto cleanup = [](vector<p2t::Point *> &points) {
    for (p2t::Point *p : points) { delete p; }
    points.clear();
  };

  const static auto trianglesToVertices = [](const vector<p2t::Triangle *> &triangles) {
    vector<Vertex> vertices;
    vertices.reserve(vertices.size() + triangles.size() * 3);
    for (auto t : triangles) {
      vertices.push_back({
        static_cast<float>(t->GetPoint(0)->x),
        static_cast<float>(t->GetPoint(0)->y),
      0});
      vertices.push_back({
        static_cast<float>(t->GetPoint(1)->x),
        static_cast<float>(t->GetPoint(1)->y),
      0});
      vertices.push_back({
        static_cast<float>(t->GetPoint(2)->x),
        static_cast<float>(t->GetPoint(2)->y),
      0});
    }
    return vertices;
  };

  vector<p2t::Point *> points;
  points.reserve(num);
  for (uint32_t i = 0; i < num; ++i) {
    points.push_back(new p2t::Point(vertices[i].x, vertices[i].y));
  }

  p2t::CDT *cdt = new p2t::CDT(points);
  cdt->Triangulate();
  vector<p2t::Triangle*> tris = cdt->GetTriangles();
  auto r = trianglesToVertices(tris);

  cleanup(points);
  delete cdt;

  return r;
}

const static std::string shaderCode = R"===(
#define PI 3.14159265359
#define TWO_PI 6.28318530718

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vec4 p = model * vertpos;

  float radius = demoloop_ScreenSize.y * 0.2;
  float left = (radius - demoloop_ScreenSize.y * 0.05) + p.y;
  // float left = exp(p.y/(radius - demoloop_ScreenSize.y * 0.05)) * 25;
  float right = p.x / radius;

  p.x = left * sin(right);
  p.y = left * cos(right);

  return transform_proj * p;
}
#endif

#ifdef PIXEL
#define NUM_COLORS 4
uniform vec3 gradientColors[NUM_COLORS];

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float c = pow(length(screen_coords / demoloop_ScreenSize.xy * 2. - 1.), 3.0);
  // float c = pow(length(screen_coords / demoloop_ScreenSize.xy * 2. - 1.), 2.0) * 0.8;

  float stepIncrement = 1.0 / float(NUM_COLORS);
  float step = 0.0;

  vec3 mixed = gradientColors[0];
  for (int i = 1; i < NUM_COLORS; ++i) {
    // mixed = mix(mixed, gradientColors[i], smoothstep(step, step + stepIncrement, c)); step += stepIncrement;
    mixed = mix(mixed, gradientColors[i], smoothstep(step, step + 0.01, c)); step += stepIncrement;
  }

  // return Texel(texture, tc) * color * vec4(mixed, 1.0);
  return Texel(texture, tc) * color;
}
#endif
)===";

const RGB colors[4] = {
  {167, 211, 226},
  {225, 246, 249},
  {255, 186, 230},
  {255, 133, 192},
};

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(CYCLE_LENGTH, 720, 720, 62,209,164), shader({shaderCode, shaderCode}) {
    glDisable(GL_DEPTH_TEST);
    // glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    // gl.getProjection() = perspective;

    auto rawVerts = rectangle<points_per_side>(640, 72);
    vertices = triangulate(rawVerts.size(), rawVerts.data());

    glm::vec3 normalizedColors[4];
    transform(begin(colors), end(colors), begin(normalizedColors), [](const RGB &c) {
      return glm::vec3(c.r / 255.0, c.g / 255.0, c.b / 255.0);
    });
    shader.sendFloat("gradientColors", 3, &normalizedColors[0].x, 4);

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});

    // m = glm::translate(m, {0, 0, 30});
  }

  ~Loop055() {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const static auto getTransform = [](const float &cycle_ratio, const float &/*width*/, const float &height) {
      glm::mat4 transform;
      transform = glm::translate(transform, {
        cosf(DEMOLOOP_M_PI * 2 * cycle_ratio) * height * 0.3,
        sinf(DEMOLOOP_M_PI * 2 * cycle_ratio) * height * 0.3,
      0});
      transform = glm::scale(transform, {
        sinf(cycle_ratio * DEMOLOOP_M_PI * 2),
        1,
      1});
      return transform;
    };

    shader.attach();

    const uint32_t num_layers = 2;
    const uint32_t indices[num_layers] = {1, 3};
    for (uint32_t i = 0; i < num_layers; ++i) {
      setColor(colors[indices[i]]);
      const float offset = static_cast<float>(i) / num_layers + DEMOLOOP_M_PI / 4;
      glm::mat4 transform = getTransform(fmod(cycle_ratio + offset, 1), width, height);
      transform = glm::scale(transform, {0.25, 0.25, 1});
      gl.triangles(vertices.data(), vertices.size(), transform);
    }

    for (uint32_t i = 0; i < num_layers; ++i) {
      setColor(colors[i * 2]);
      const float offset = static_cast<float>(i) / num_layers;
      glm::mat4 transform = getTransform(fmod(cycle_ratio + offset, 1), width, height);
      gl.triangles(vertices.data(), vertices.size(), transform);
    }

    shader.detach();
  }

private:
  const static uint32_t points_per_side = 60;
  vector<Vertex> vertices;
  Shader shader;
};

int main(int, char**){
  Loop055 test;
  test.Run();

  return 0;
}
