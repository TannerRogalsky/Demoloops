#include <array>
#include <vector>
#include <unordered_map>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include "graphics/shader.h"
#include "demoloop.h"
using namespace std;
using namespace demoloop;

const glm::vec3 twoDAxis = {0, 0, 1};

const uint32_t CYCLE_LENGTH = 10;
const uint32_t MAX_VERTS = 12;
const float SPATIAL_HASH_SIZE = 100;
array<array<glm::vec4, MAX_VERTS>, MAX_VERTS - 3> polygonVerts;

float randFloat() {
  return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

template<uint32_t N>
typename std::enable_if<N >= 3, array<Vertex, N>>::type
polygonVertices(const float &radius) {
  array<Vertex, N> r;

  const float interval = DEMOLOOP_M_PI * 2 / N;
  float phi = 0.0f;
  for (uint32_t i = 0; i < N; ++i, phi += interval) {
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

template<typename K, typename V>
struct PairHasher {
  std::size_t operator()(const pair<K, V> &k) const {
    return hash<K>()(k.first) ^ (hash<V>()(k.second) << 1);
  }
};

struct Polygon {
  uint32_t num_verts;
  glm::mat4 transform;
};

typedef unordered_map<pair<int32_t, int32_t>, vector<Polygon>, PairHasher<int32_t, int32_t>> SpatialHash;

struct TreeData {
  vector<vector<Polygon>> tree;
  vector<Polygon> all_shapes;
  uint32_t previous_layer;
  uint32_t previous_shape;
  unordered_map<uint32_t, vector<glm::mat4>> shapes_by_vertex_count;
  SpatialHash spatial_hash;
};

bool doShapesIntersect(const float radius, const Polygon &a, const Polygon &b) {
  glm::vec2 translationA(a.transform[3]);
  glm::vec2 translationB(b.transform[3]);

  float scaleA = glm::fastSqrt(a.transform[0].x * a.transform[0].x + a.transform[0].y * a.transform[0].y);
  float scaleB = glm::fastSqrt(b.transform[0].x * b.transform[0].x + b.transform[0].y * b.transform[0].y);

  float x1 = translationA.x, y1 = translationA.y;
  float x2 = translationB.x, y2 = translationB.y;

  uint32_t v1 = a.num_verts, v2 = b.num_verts;
  float r1 = radius * scaleA * cosf(DEMOLOOP_M_PI / v1), r2 = radius * scaleB * cosf(DEMOLOOP_M_PI / v2);

  float dx = x1 - x2, dy = y1 - y2;
  float d = dx * dx + dy * dy;
  float i = r1 + r2 - 0.1;

  bool inscriptions_overlap = d < (i * i);
  if (inscriptions_overlap) {
    return true;
  }

  // const array<glm::vec4, MAX_VERTS> &verticesA = polygonVerts[v1 - 3];
  // const array<glm::vec4, MAX_VERTS> &verticesB = polygonVerts[v2 - 3];

  // // printf("%u\n", v1);
  // for (uint32_t k = 0; k < v2; ++k) {
  //   const glm::vec4 v = verticesB[k] * b.transform;

  //   bool result = false;
  //   uint32_t j = v1 - 1;
  //   for (uint32_t i = 0; i < v1; ++i) {
  //     // printf("%u, %u\n", i, j);
  //     glm::vec4 e1 = verticesA[j] * a.transform;
  //     glm::vec4 e2 = verticesA[i] * a.transform;
  //     printf("%f > %f\n", e1.y, v.y);

  //     // if ((e1.y > v.y) != (e2.y > v.y) && (v.x < (e2.x - e1.x) * (v.y - e1.y) / (e2.y-e1.y) + e1.x)) {
  //     // if (e1.y > v.y) {
  //     //   result = !result;
  //     // }

  //     j = i;
  //   }

  //   if (result) {
  //     return true;
  //   }
  // }
  // printf("done\n");

  return false;
}

bool intersectsAny(const float radius, const Polygon &shape, const vector<Polygon> &shapes) {
  for (const Polygon &other : shapes) {
    if (doShapesIntersect(radius, shape, other)) {
      return true;
    }
  }
  return false;
}

void populateSpatialHash(const Polygon &p, const pair<int32_t, int32_t> &base, SpatialHash &spatial_hash) {
  spatial_hash[base].push_back(p);
  spatial_hash[{base.first + 1, base.second}].push_back(p);
  spatial_hash[{base.first - 1, base.second}].push_back(p);
  spatial_hash[{base.first, base.second + 1}].push_back(p);
  spatial_hash[{base.first, base.second - 1}].push_back(p);
  spatial_hash[{base.first + 1, base.second + 1}].push_back(p);
  spatial_hash[{base.first + 1, base.second - 1}].push_back(p);
  spatial_hash[{base.first - 1, base.second - 1}].push_back(p);
  spatial_hash[{base.first - 1, base.second + 1}].push_back(p);
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

  float PI = DEMOLOOP_M_PI;

  for (uint32_t i = 0; i < previous_vertex_count; ++i) {
    uint32_t current_vertex_count = randFloat() * (MAX_VERTS - 3) + 3;
    float current_side_length = sinf(DEMOLOOP_M_PI / current_vertex_count) * 2 * radius;

    float t = (DEMOLOOP_M_PI * 2) / previous_vertex_count;
    float inner_outer_ratio = previous_side_length / current_side_length;

    float previous_distance = radius * cosf(DEMOLOOP_M_PI / previous_vertex_count);
    float current_distance = radius * cosf(DEMOLOOP_M_PI / current_vertex_count) * inner_outer_ratio;
    float d = previous_distance + current_distance;

    glm::mat4 transform = previous_shape.transform;
    transform = glm::rotate(transform, i * t, twoDAxis);
    transform = glm::rotate(transform, PI / previous_vertex_count, twoDAxis);
    transform = glm::translate(transform, {d, 0, 0});
    transform = glm::rotate(transform, PI / current_vertex_count + PI, twoDAxis);
    transform = glm::scale(transform, {inner_outer_ratio, inner_outer_ratio, 1});

    Polygon p = {current_vertex_count, transform};
    pair<int32_t, int32_t> hash_lookup({floor(transform[3][0] / SPATIAL_HASH_SIZE), floor(transform[3][1] / SPATIAL_HASH_SIZE)});
    if (intersectsAny(radius, p, treeData.spatial_hash[hash_lookup]) == false) {
      current_layer.push_back(p);
      all_shapes.push_back(p);
      treeData.shapes_by_vertex_count[p.num_verts].push_back(p.transform);
      populateSpatialHash(p, hash_lookup, treeData.spatial_hash);
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
    addForShape(radius, treeData);
  }
}

TreeData build(const float radius, const uint32_t num_layers) {
  vector<vector<Polygon>> tree;
  vector<Polygon> all_shapes;
  unordered_map<uint32_t, vector<glm::mat4>> shapes_by_vertex_count;
  SpatialHash spatial_hash;

  {
    Polygon p = {6, glm::mat4()};
    tree.push_back({p});
    all_shapes.push_back(p);
    shapes_by_vertex_count[p.num_verts].push_back(p.transform);
    pair<int32_t, int32_t> hash_lookup = {0, 0};
    populateSpatialHash(p, hash_lookup, spatial_hash);
  }

  TreeData treeData = {tree, all_shapes, 0, 0, shapes_by_vertex_count, spatial_hash};

  for (uint32_t layer_index = 1; layer_index < num_layers; ++layer_index) {
    addLayer(radius, treeData);
  }

  return treeData;
}

uint32_t bufferMatrixAttribute(const GLint location, const GLuint buffer, const float *data, const uint32_t num) {
  uint32_t enabledAttribs = 0;
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, num * sizeof(float) * 4 * 4, data, GL_DYNAMIC_DRAW);

  for (int i = 0; i < 4; ++i) {
    enabledAttribs |= 1u << (uint32_t)(location + i);
    glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*)(sizeof(float) * (4 * i)));
    glVertexAttribDivisor(location + i, 1);
  }

  return enabledAttribs;
}

const static std::string shaderCode = R"===(
varying vec4 vpos;
// varying vec4 vColor;

#ifdef VERTEX
attribute mat4 modelViews;
// attribute vec4 colors;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  // vColor = colors;
  return transform_proj * modelViews * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords) {
  return Texel(texture, texture_coords) * color;
}
#endif
)===";

class Loop021 : public Demoloop {
public:
  Loop021() : Demoloop(CYCLE_LENGTH, 720, 720, 150, 150, 150), RADIUS(height / 20), shader({shaderCode, shaderCode}) {
    glDisable(GL_DEPTH_TEST);

    glGenBuffers(MAX_VERTS - 3, vbos);
    buffer<MAX_VERTS>(RADIUS, vbos);

    for (uint32_t N = 3; N < MAX_VERTS; ++N) {
      auto &r = polygonVerts[N - 3];
      const float interval = DEMOLOOP_M_PI * 2 / N;
      float phi = 0.0f;
      for (uint32_t i = 0; i < N; ++i, phi += interval) {
        r[i].x = cosf(phi) * RADIUS;
        r[i].y = sinf(phi) * RADIUS;
        r[i].z = 0;
        r[i].w = 0;
      }
    }

    glGenBuffers(1, &modelViewsBuffer);
    glGenBuffers(1, &colorsBuffer);

    uint32_t seed = time(0);
    seed = 1485781690;
    printf("%u\n", seed);
    srand(seed);
    treeData = build(RADIUS, 5);
  }

  ~Loop021() {
    glDeleteBuffers(MAX_VERTS - 3, vbos);
    glDeleteBuffers(1, &modelViewsBuffer);
    glDeleteBuffers(1, &colorsBuffer);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    // for (uint32_t i = 0; i < 35; ++i) {
    //   addForShape(RADIUS, treeData);
    // }
    addForShape(RADIUS, treeData);

    float scale = 0.7 * (1-powf(sinf(cycle_ratio*DEMOLOOP_M_PI),2)*0.55);
    scale = 0.8;
    GL::TempTransform t1(gl);
    t1.get() = glm::scale(glm::translate(t1.get(),
      glm::vec3(width / 2, height / 2, 0)), glm::vec3(scale, scale, 1)
    );

    shader.attach();
    gl.prepareDraw();

    uint32_t modelViewsLocation = shader.getAttribLocation("modelViews");
    for(const auto &shapes : treeData.shapes_by_vertex_count) {

      uint32_t enabledAttribs = bufferMatrixAttribute(modelViewsLocation, modelViewsBuffer, &shapes.second.data()[0][0][0], shapes.second.size());

      glBindBuffer(GL_ARRAY_BUFFER, vbos[shapes.first - 3]);
      glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
      // glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

      gl.useVertexAttribArrays(enabledAttribs | ATTRIBFLAG_POS);

      setColor(hsl2rgb((float)shapes.first/MAX_VERTS, 1, 0.5));
      // gl.drawArraysInstanced(GL_TRIANGLE_FAN, 0, shapes.first, shapes.second.size());
      // setColor(0, 0, 0);
      gl.drawArraysInstanced(GL_LINE_LOOP, 0, shapes.first, shapes.second.size());
    }

    shader.detach();

    // setColor(0, 0, 0);
    // gl.lineLoop(polygonVerts[3].data(), 6, glm::mat4());
  }
private:
  const float RADIUS;
  Shader shader;
  GLuint vbos[MAX_VERTS - 3];
  TreeData treeData;
  GLuint modelViewsBuffer;
  GLuint colorsBuffer;
};

int main(int, char**){
  Loop021 loop;
  loop.Run();

  return 0;
}
