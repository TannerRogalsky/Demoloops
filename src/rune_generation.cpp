#include "demoloop.h"
#include "graphics/2d_primitives.h"
#include "graphics/shader.h"
#include "helpers.h"
#include <math.h>
#include "math_helpers.h"
#include <array>
#include <vector>
#include <glm/gtx/rotate_vector.hpp>
#include "poly2tri.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

typedef vector<p2t::Point *> PointsList;

const static std::string shaderCode = R"===(
#define PI 3.14159265359
#define TWO_PI 6.28318530718

#ifdef VERTEX
vec2 rotate(vec2 pos, float rad){
  return .5 + (pos-.5) * mat2(cos(rad), -sin(rad), sin(rad), cos(rad));
}

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  // vertpos.xy = rotate(vertpos.xy, length(vertpos.xy) * 0.1);
  vertpos.xy = rotate(vertpos.xy, 0.1);
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  return Texel(texture, tc) * color;
}
#endif
)===";

void trianglesToVertices(vector<Vertex> &vertices, const vector<p2t::Triangle *> &triangles) {
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
}

void cleanup(PointsList &points) {
  for (p2t::Point *p : points) {
    delete p;
  }
  points.clear();
}

vector<Vertex> triangulate(const vector<Vertex> &vertices) {
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

  const static auto verticesToPoints = [](const vector<Vertex> &vertices) {
    vector<p2t::Point *> points;
    points.reserve(vertices.size());
    for (const Vertex &v : vertices) {
      points.push_back(new p2t::Point(v.x, v.y));
    }
    return points;
  };

  const static auto addHole = [](auto&& self, p2t::CDT *cdt, const vector<Vertex> &hole, auto&&... rest) {
    printf("hey\n");
    self(cdt, rest...);
  };

  vector<p2t::Point *> points = verticesToPoints(vertices);

  p2t::CDT *cdt = new p2t::CDT(points);
  addHole(addHole, cdt, vertices);
  cdt->Triangulate();
  vector<p2t::Triangle*> tris = cdt->GetTriangles();
  auto r = trianglesToVertices(tris);

  cleanup(points);
  delete cdt;

  return r;
}

float lerp(float v0, float v1, float t) {
  return v0 + t * (v1 - v0);
}

float randFloat() {
  return static_cast<float> (rand()) / static_cast <float> (RAND_MAX);
}

PointsList generateEllipse(const float width, const float height) {
  const uint32_t points_per_ellipse = 60;
  const float ellipse_interval = DEMOLOOP_M_PI * 2 / points_per_ellipse;

  PointsList points;
  points.reserve(points_per_ellipse);
  for (uint32_t i = 0; i < points_per_ellipse; ++i) {
    float x = cosf(i * ellipse_interval) * width;
    float y = sinf(i * ellipse_interval) * height;

    points.push_back(new p2t::Point(x, y));
  }

  return points;
}

PointsList generateRectangle(const float width, const float height) {
  const uint32_t points_per_side = 3;
  PointsList points;
  points.reserve(points_per_side * 4);

  for (uint32_t i = 0; i < points_per_side; ++i) {
    points.push_back(new p2t::Point(-width + width * 2 * static_cast<float>(i) / points_per_side, -height));
  }
  for (uint32_t i = 0; i < points_per_side; ++i) {
    points.push_back(new p2t::Point(width, -height + height * 2 * static_cast<float>(i) / points_per_side));
  }
  for (uint32_t i = 0; i < points_per_side; ++i) {
    points.push_back(new p2t::Point(width - (width * 2) * static_cast<float>(i) / points_per_side, height));
  }
  for (uint32_t i = 0; i < points_per_side; ++i) {
    points.push_back(new p2t::Point(-width, height - height * 2 * static_cast<float>(i) / points_per_side));
  }

  return points;
}

PointsList generateGlyph(const float width, const float height) {
  // return generateEllipse(width, height);
  return generateRectangle(width, height);
}

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(1280, 720, 255,119,170), shader({shaderCode, shaderCode}) {
    glDisable(GL_DEPTH_TEST);

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});

    srand(1); randFloat();

    const float radius = height * 0.4;
    const float thickness = radius * 0.1;

    const uint32_t num_verts = 6;
    const float interval = DEMOLOOP_M_PI * 2 / num_verts;
    const float d = radius * sinf(DEMOLOOP_M_PI / num_verts);

    for (uint32_t i = 0; i < num_verts; ++i) {
      const float ec = cosf(i * interval - DEMOLOOP_M_PI / 2);
      const float es = sinf(i * interval - DEMOLOOP_M_PI / 2);

      PointsList points = generateGlyph(d, thickness);
      for (p2t::Point *p : points) {
        // shape glyph around the circle
        const float left = radius + p->y; // radius
        const float right = p->x / (radius); // theta

        // from polar
        p->x = left * sinf(right);
        p->y = left * cosf(right);

        // rotate shape into position
        const float rx = ec*p->x - es*p->y;
        const float ry = es*p->x + ec*p->y;

        p->x = rx;
        p->y = ry;
      }

      p2t::CDT *cdt = new p2t::CDT(points);
      cdt->Triangulate();
      vector<p2t::Triangle*> tris = cdt->GetTriangles();
      trianglesToVertices(vertices, tris);

      cleanup(points);
      delete cdt;
    }
  }

  ~Loop055() {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    setColor(119,170,255);
    circle(gl, 0, 0, height * 0.4, 60);

    {
      // shader.attach();
      setColor(0, 0, 0, 200);
      glm::mat4 transform;
      transform = glm::translate(transform, {0, 0, 1});
      // transform = glm::rotate(transform, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0, 0, 1});
      gl.triangles(vertices.data(), vertices.size(), transform);
      // shader.detach();
    }

    // {
    //   const uint32_t num_lines = 12;
    //   const float interval = DEMOLOOP_M_PI * 2 / num_lines;
    //   setColor(255, 255, 255);
    //   for (uint32_t i = 0; i < num_lines; ++i) {
    //     line(gl, 0, 0, cosf(i * interval) * height / 2, sinf(i * interval) * height / 2);
    //   }
    // }
  }

private:
  Shader shader;
  vector<Vertex> vertices;
  vector<Vertex> glyph;
};

int main(int, char**){
  Loop055 test;
  test.Run();

  return 0;
}
