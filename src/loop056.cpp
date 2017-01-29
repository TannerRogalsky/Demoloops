#include <array>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "demoloop.h"
#include "hsl.h"
#include "math_helpers.h"
using namespace std;
using namespace demoloop;

const glm::vec3 twoDAxis = {0, 0, 1};
float t = 0;
const float CYCLE_LENGTH = 10;
const uint32_t MAX_VERTS = 12;

float randFloat() {
  return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

template<uint N>
typename std::enable_if<N >= 3, array<Vertex, N>>::type
polygonVertices(const float &radius) {
  array<Vertex, N> r;

  const float interval = DEMOLOOP_M_PI * 2 / N;
  float phi = 0.0f;
  // float phi = rotationOffset(N);
  for (uint i = 0; i < N; ++i, phi += interval) {
    r[i].x = cosf(phi) * radius;
    r[i].y = sinf(phi) * radius;
    r[i].z = 0;

    // RGB c = hsl2rgb(phi / (DEMOLOOP_M_PI * 2), 1, 0.5);
    // r[i].r = c.r;
    // r[i].g = c.g;
    // r[i].b = c.b;
  }

  return r;
}

template<uint32_t N, uint32_t I = 3>
typename std::enable_if<I == N, void>::type
buffer(float, GLuint*) {}

template<uint32_t N, uint32_t I = 3>
typename std::enable_if<I < N, void>::type
buffer(float RADIUS, GLuint *vbos) {
  const array<Vertex, I> vertex_data = polygonVertices<I>(RADIUS);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[I - 3]);
  glBufferData(GL_ARRAY_BUFFER, I * sizeof(Vertex), vertex_data.data(), GL_DYNAMIC_DRAW);
  buffer<N, I + 1>(RADIUS, vbos);
}

struct Polygon {
  uint32_t num_verts;
  glm::mat4 transform;
};

struct TreeData {
  vector<vector<Polygon>> tree;
  vector<Polygon> all_shapes;
  uint32_t previous_layer;
  uint32_t previous_shape;
};

bool doShapesIntersect(const float radius, const Polygon &a, const Polygon &b) {
  glm::vec3 translationA(a.transform[3]);
  glm::vec3 translationB(b.transform[3]);
  float scaleA = glm::length(a.transform[0]);
  float scaleB = glm::length(b.transform[0]);

  float x1 = translationA.x, y1 = translationA.y;
  float x2 = translationB.x, y2 = translationB.y;

  uint32_t v1 = a.num_verts, v2 = b.num_verts;
  float r1 = radius * scaleA * cosf(DEMOLOOP_M_PI / v1), r2 = radius * scaleB * cosf(DEMOLOOP_M_PI / v2);

  float dx = x1 - x2, dy = y1 - y2;
  float d = sqrt(dx * dx + dy * dy);

  bool inscriptions_overlap = d < (r1 + r2 - 0.1);
  return inscriptions_overlap;
}

bool intersectsAny(const float radius, const Polygon &shape, const vector<Polygon> &shapes) {
  for (const Polygon &other : shapes) {
    if (doShapesIntersect(radius, shape, other)) {
      return true;
    }
  }
  return false;
}

void addForShape(const float radius, TreeData &treeData) {
  auto &tree = treeData.tree;
  auto &all_shapes = treeData.all_shapes;

  if (tree.size() == treeData.previous_layer + 1) {
    tree.push_back({});
  }

  vector<Polygon> &previous_layer = tree[treeData.previous_layer];
  vector<Polygon> &current_layer = tree[treeData.previous_layer + 1];

  const Polygon &previous_shape = previous_layer[treeData.previous_shape];
  uint32_t previous_vertex_count = previous_shape.num_verts;
  float previous_side_length = sinf(DEMOLOOP_M_PI / previous_vertex_count) * 2 * radius;
  // printf("previous_vertex_count: %u\n", previous_vertex_count);

  float PI = DEMOLOOP_M_PI;

  for (uint32_t i = 0; i < previous_vertex_count; ++i) {
    uint32_t current_vertex_count = randFloat() * (MAX_VERTS - 3) + 3;
    float current_side_length = sinf(DEMOLOOP_M_PI / current_vertex_count) * 2 * radius;
    // printf("current_vertex_count: %u\n", current_vertex_count);

    float t = (DEMOLOOP_M_PI * 2) / previous_vertex_count;
    float inner_outer_ratio = previous_side_length / current_side_length;
    // printf("inner_outer_ratio: %f\n", inner_outer_ratio);

    float previous_distance = radius * cosf(DEMOLOOP_M_PI / previous_vertex_count);
    float current_distance = radius * cosf(DEMOLOOP_M_PI / current_vertex_count) * inner_outer_ratio;
    float d = previous_distance + current_distance;
    // printf("%f\n", d);

    glm::mat4 transform = previous_shape.transform;
    transform = glm::rotate(transform, i * t, twoDAxis);
    transform = glm::rotate(transform, PI / previous_vertex_count, twoDAxis);
    transform = glm::translate(transform, {d, 0, 0});
    transform = glm::rotate(transform, PI / current_vertex_count + PI, twoDAxis);
    transform = glm::scale(transform, {inner_outer_ratio, inner_outer_ratio, 1});

    Polygon p = {current_vertex_count, transform};
    if (intersectsAny(radius, p, all_shapes) == false) {
      current_layer.push_back(p);
      all_shapes.push_back(p);
    }
  }

  treeData.previous_shape++;
  if (treeData.previous_shape == previous_layer.size()) {
    treeData.previous_layer++;
    treeData.previous_shape = 0;
  }
}

void addLayer(const float radius, TreeData &treeData) {
  for (uint32_t i = 0; i < treeData.tree[treeData.previous_layer].size(); ++i) {
    // printf("layer_index %u, shape_index: %u\n", treeData.previous_layer, treeData.previous_shape);
    addForShape(radius, treeData);
  }
}

TreeData build(const float radius, const uint32_t num_layers) {
  vector<vector<Polygon>> tree;
  vector<Polygon> all_shapes;

  {
    Polygon p = {6, glm::mat4()};
    tree.push_back({p});
    all_shapes.push_back(p);
  }

  TreeData treeData = {tree, all_shapes, 0, 0};

  for (uint32_t layer_index = 1; layer_index < num_layers; ++layer_index) {
    addLayer(radius, treeData);
  }

  return treeData;
}

class Loop021 : public Demoloop {
public:
  Loop021() : Demoloop(720, 720, 150, 150, 150), RADIUS(height / 20) {
    glDisable(GL_DEPTH_TEST);

    glGenBuffers(MAX_VERTS - 3, vbos);
    buffer<MAX_VERTS>(RADIUS, vbos);

    treeData = build(RADIUS, 6);
  }

  ~Loop021() {
    glDeleteBuffers(MAX_VERTS - 3, vbos);
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    for (uint32_t i = 0; i < 25; ++i) {
      addForShape(RADIUS, treeData);
    }
    // addForShape(RADIUS, treeData);

    float scale = 0.25 * (1-cycle_ratio*0.75);
    GL::TempTransform t1(gl);
    t1.get() = glm::scale(glm::translate(t1.get(),
      glm::vec3(width / 2, height / 2, 0)), glm::vec3(scale, scale, 1)
    );

    uint32_t index = 0;
    for(const vector<Polygon> &layer : treeData.tree) {
      setColor(hsl2rgb((float)index/treeData.tree.size(), 1, 0.5));
      for(const Polygon &p : layer) {
        gl.prepareDraw(p.transform);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[p.num_verts - 3]);
        gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR);
        glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
        glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));
        gl.drawArrays(GL_TRIANGLE_FAN, 0, p.num_verts);
      }
      index++;
    }
  }
private:
  const float RADIUS;
  GLuint vbos[MAX_VERTS - 3];
  TreeData treeData;
};

int main(int, char**){
  Loop021 loop;
  loop.Run();

  return 0;
}
