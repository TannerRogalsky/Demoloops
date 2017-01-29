#include <array>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "demoloop.h"
#include "hsl.h"
#include "math_helpers.h"
using namespace std;
using namespace demoloop;

const glm::vec3 twoDAxis = {0, 0, 1};
float t = 0;
const float CYCLE_LENGTH = 5;
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

    RGB c = hsl2rgb(phi / (DEMOLOOP_M_PI * 2), 1, 0.5);
    r[i].r = c.r;
    r[i].g = c.g;
    r[i].b = c.b;
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

bool doShapesIntersect(const float radius, const Polygon &a, const Polygon &b) {
  static glm::vec3 scaleA;
  static glm::vec3 translationA;
  static glm::vec3 scaleB;
  static glm::vec3 translationB;

  static glm::quat rotation;
  static glm::vec3 skew;
  static glm::vec4 perspective;
  glm::decompose(a.transform, scaleA, rotation, translationA, skew, perspective);
  glm::decompose(b.transform, scaleB, rotation, translationB, skew, perspective);

  float x1 = translationA.x, y1 = translationA.y;
  float x2 = translationB.x, y2 = translationB.y;

  uint32_t v1 = a.num_verts, v2 = b.num_verts;
  float r1 = radius * scaleA.x * cosf(DEMOLOOP_M_PI / v1), r2 = radius * scaleB.x * cosf(DEMOLOOP_M_PI / v2);

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

vector<vector<Polygon>> build(const float radius, const uint32_t num_layers) {
  vector<vector<Polygon>> tree(num_layers);
  vector<Polygon> all_shapes;

  {
    Polygon p = {3, glm::mat4()};
    tree[0].push_back(p);
    all_shapes.push_back(p);
  }

  for (uint32_t layer_index = 1; layer_index < num_layers; ++layer_index) {
    // printf("layer_index: %u\n", layer_index);
    vector<Polygon> &previous_layer = tree[layer_index - 1];
    vector<Polygon> &current_layer = tree[layer_index];

    Polygon previous_shape = previous_layer[0];
    uint32_t previous_vertex_count = previous_shape.num_verts;
    float previous_side_length = sinf(DEMOLOOP_M_PI / previous_vertex_count) * 2 * radius;
    // printf("previous_vertex_count: %u\n", previous_vertex_count);

    uint32_t current_vertex_count = layer_index + 3;
    current_vertex_count = randFloat() * (MAX_VERTS - 3) + 3;
    float current_side_length = sinf(DEMOLOOP_M_PI / current_vertex_count) * 2 * radius;
    // printf("current_vertex_count: %u\n", current_vertex_count);

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(previous_shape.transform, scale, rotation, translation, skew, perspective);

    float t = (DEMOLOOP_M_PI * 2) / previous_vertex_count;
    float inner_outer_ratio = previous_side_length / current_side_length;
    // printf("inner_outer_ratio: %f\n", inner_outer_ratio);

    float previous_distance = radius * cosf(DEMOLOOP_M_PI / previous_vertex_count);
    float current_distance = radius * cosf(DEMOLOOP_M_PI / current_vertex_count) * inner_outer_ratio;
    float d = previous_distance + current_distance;
    // printf("%f\n", d);

    float PI = DEMOLOOP_M_PI;

    for (uint32_t i = 0; i < previous_vertex_count; ++i) {
      for (const Polygon &previous_shape : previous_layer) {
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
    }
    // printf("\n");
  }

  return tree;
}

class Loop021 : public Demoloop {
public:
  Loop021() : Demoloop(1280, 720, 150, 150, 150), RADIUS(height / 20) {
    glDisable(GL_DEPTH_TEST);

    glGenBuffers(MAX_VERTS - 3, vbos);
    buffer<MAX_VERTS>(RADIUS, vbos);

    auto start = std::chrono::high_resolution_clock::now();
    tree = build(RADIUS, 10);
    auto delta = std::chrono::high_resolution_clock::now() - start;
    printf("built in %f\n", std::chrono::duration_cast<std::chrono::duration<float>>(delta).count());
  }

  ~Loop021() {
    glDeleteBuffers(MAX_VERTS - 3, vbos);
  }

  void Update(float dt) {
    t += dt;

    float cycle = fmod(t, CYCLE_LENGTH);
    float cycle_ratio = cycle / CYCLE_LENGTH;

    float scale = 1-pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2)*0.99;
    GL::TempTransform t1(gl);
    t1.get() = glm::scale(glm::translate(t1.get(),
      glm::vec3(width / 2, height / 2, 0)), glm::vec3(scale, scale, 1)
    );

    for(const vector<Polygon> &layer : tree) {
      for(const Polygon &p : layer) {
        gl.prepareDraw(p.transform);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[p.num_verts - 3]);
        gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR);
        glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
        glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));
        gl.drawArrays(GL_TRIANGLE_FAN, 0, p.num_verts);
      }
    }

    // const uint32_t N = jmap(cycle_ratio, 0, 1, 3, MAX_VERTS);

    // gl.prepareDraw();
    // glBindBuffer(GL_ARRAY_BUFFER, vbos[N - 3]);
    // gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR);
    // glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    // glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));
    // gl.drawArrays(GL_TRIANGLE_FAN, 0, N);
  }
private:
  const float RADIUS;
  GLuint vbos[MAX_VERTS - 3];
  vector<vector<Polygon>> tree;
};

int main(int, char**){
  Loop021 loop;
  loop.Run();

  return 0;
}
