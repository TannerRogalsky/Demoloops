#include "demoloop.h"
#include "graphics/shader.h"
#include "math_helpers.h"
#include "graphics/polyline.h"
#include <array>
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 3;

const static std::string shaderCode = R"===(
#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
uniform float cycle_ratio;

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  // float height = 0.1 + 0.1 * fract(sin((tc.y) * 3.14) * 100000.0);
  float height = 0.1;
  float c = smoothstep(height, height + 0.1, tc.x) * smoothstep(height, height + 0.1, 1.0 - tc.x);

  return Texel(texture, tc) * color * vec4(vec3(c), 1.0);
}
#endif
)===";

const static std::string roseShaderCode = R"===(
#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 /*color*/, Image texture, vec2 tc, vec2 screen_coords) {
  vec2 pos = vec2(0.5)-tc;

  float r = length(pos)*2.1;
  float a = atan(pos.y,pos.x);

  float f = pow(cos(a*2.5), 0.15);
  vec4 color = vec4(0.0);
  color.a = 1.0 - smoothstep(f,f+0.07,r);

  return color;
}
#endif
)===";

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

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(720, 720, 150, 150, 150), shader({shaderCode, shaderCode}), roseShader({roseShaderCode, roseShaderCode}) {
    // glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    // gl.getProjection() = perspective;

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
  }

  ~Loop055() {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;


    const uint32_t NUM_VERTS = 7;
    const uint32_t NUM_SIDES = 1;
    glm::vec2 linePoints[NUM_VERTS];
    const float interval = DEMOLOOP_M_PI * 2 / (NUM_VERTS - 1);
    const float RADIUS = height * 0.4;
    const float THICKNESS = 0.05;

    const uint32_t indices[NUM_VERTS] = {0, 2, 5, 3, 1, 4};

    uint32_t index = 0;
    for (uint32_t i = 0; i < NUM_VERTS; ++i) {
      const uint32_t a = indices[i];
      const float phi1 = interval * a;

      for (uint32_t j = 0; j < NUM_SIDES; ++j) {

        linePoints[index].x = cosf(phi1) * RADIUS;
        linePoints[index].y = sinf(phi1) * RADIUS;
        index++;
      }
    }

    glm::mat4 view = glm::mat4();
    view = glm::rotate(view, (float)DEMOLOOP_M_PI / 2, {0, 0, 1});

    GL::TempTransform t1(gl);
    t1.get() = glm::translate(t1.get(), {0, 0, 1});
    polyline.render((float *)linePoints, NUM_VERTS * 2, RADIUS * THICKNESS, 1, false);
    // polyline.draw();

    const uint32_t num_v = NUM_VERTS * 2;
    const uint32_t num_edges = NUM_VERTS - 1;
    const uint32_t points_per_edge = 3;
    const uint32_t total_verts = num_v + num_edges * 2 * points_per_edge;
    Vertex vertices[total_verts];

    // set vertex positions
    for (uint32_t i = 0; i < NUM_VERTS; ++i) {
      float t = static_cast<float>(i) / (NUM_VERTS - 1);
      uint32_t line_index1 = i * 2;
      uint32_t line_index2 = line_index1 + 1;

      uint32_t v_index = i * 2 + i * 2 * points_per_edge;
      Vertex &v1 = vertices[v_index];
      Vertex &v2 = vertices[v_index + 1];

      v1.x = polyline.vertices[line_index1].x;
      v1.y = polyline.vertices[line_index1].y;
      v1.z = 10;
      v1.s = 1;
      v1.t = t;

      v2.x = polyline.vertices[line_index2].x;
      v2.y = polyline.vertices[line_index2].y;
      v2.z = 10;
      v2.s = 0;
      v2.t = t;

      applyColor(v1, hsl2rgb(fmod(t + cycle_ratio, 1.0), 1.0, 0.5));
      applyColor(v2, hsl2rgb(fmod(t + cycle_ratio, 1.0), 1.0, 0.5));
    }

    const static auto getIndex = [](const uint32_t &i, const uint32_t &points_per_edge) {
      return i * 2 + i * 2 * points_per_edge;
    };

    // set interpolated verts
    for (uint32_t i = 0; i < NUM_VERTS; ++i) {
      uint32_t v_index = getIndex(i, points_per_edge);
      Vertex &v1 = vertices[v_index];
      Vertex &v2 = vertices[v_index + 1];

      if (i != NUM_VERTS - 1) {
        const uint32_t v_n_index = getIndex(i + 1, points_per_edge);

        Vertex &v3 = vertices[v_n_index];
        Vertex &v4 = vertices[v_n_index + 1];

        for (uint32_t j = 0; j < points_per_edge; ++j) {
          const float mix_ratio = static_cast<float>(j + 1) / (points_per_edge + 1);

          const float x1 = mix(v1.x, v3.x, mix_ratio);
          const float y1 = mix(v1.y, v3.y, mix_ratio);

          const float x2 = mix(v2.x, v4.x, mix_ratio);
          const float y2 = mix(v2.y, v4.y, mix_ratio);

          const float z = glm::clamp(sin(mix_ratio * 3 * DEMOLOOP_M_PI + i * DEMOLOOP_M_PI) * 1000, -1.0, 1.0);

          vertices[v_index + (j + 1) * 2].x = x1;
          vertices[v_index + (j + 1) * 2].y = y1;
          vertices[v_index + (j + 1) * 2].z = z + 10;
          vertices[v_index + (j + 1) * 2].s = mix(v1.s, v3.s, mix_ratio);
          vertices[v_index + (j + 1) * 2].t = mix(v1.t, v3.t, mix_ratio);

          vertices[v_index + (j + 1) * 2 + 1].x = x2;
          vertices[v_index + (j + 1) * 2 + 1].y = y2;
          vertices[v_index + (j + 1) * 2 + 1].z = z + 10;
          vertices[v_index + (j + 1) * 2 + 1].s = mix(v2.s, v4.s, mix_ratio);
          vertices[v_index + (j + 1) * 2 + 1].t = mix(v2.t, v4.t, mix_ratio);

          const float t = static_cast<float>(v_index + (j + 1) * 2) / total_verts;
          applyColor(vertices[v_index + (j + 1) * 2], hsl2rgb(fmod(t + cycle_ratio, 1.0), 1.0, 0.5));
          applyColor(vertices[v_index + (j + 1) * 2 + 1], hsl2rgb(fmod(t + cycle_ratio, 1.0), 1.0, 0.5));
        }
      }
    }

    setColor(50, 50, 50);
    gl.triangleFan(polygonVertices<6>(RADIUS + RADIUS * THICKNESS).data(), 6, view);

    gl.bufferVertices(vertices, total_verts, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid *) offsetof(Vertex, r));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));

    gl.bindTexture(gl.getDefaultTexture());
    gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD);

    shader.attach();
    shader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    gl.prepareDraw(view);
    setColor(255, 255, 255);
    gl.drawArrays(GL_TRIANGLE_STRIP, 0, total_verts);

    shader.detach();

    {
      const float size = height * 0.1;
      Vertex v[4] = {
        {-size / 2, -size / 2, 0, 0, 0},
        {size / 2, -size / 2, 0, 1, 0},
        {size / 2, size / 2, 0, 1, 1},
        {-size / 2, size / 2, 0, 0, 1}
      };

      glm::mat4 transform;
      roseShader.attach();

      transform = glm::translate(transform, {
        cosf(cycle_ratio * DEMOLOOP_M_PI * 2) * height * 0.15,
        // sinf(cycle_ratio * DEMOLOOP_M_PI * 2),
        0.0,
        sinf(cycle_ratio * DEMOLOOP_M_PI * 2) * 5 + 10,
      });
      transform = glm::rotate(transform, cycle_ratio * (float)DEMOLOOP_M_PI * 2, {0, 0, 1});
      gl.triangleFan(v, 4, transform);

      transform = glm::translate(glm::mat4(), {
        0.0,
        sinf(cycle_ratio * DEMOLOOP_M_PI * 2) * height * 0.15,
        cosf(cycle_ratio * DEMOLOOP_M_PI * 2) * 5 + 10,
      });
      transform = glm::rotate(transform, cycle_ratio * (float)DEMOLOOP_M_PI * 2, {0, 0, 1});
      gl.triangleFan(v, 4, transform);

      roseShader.detach();
    }
  }

private:
  Shader shader;
  Shader roseShader;
  MiterJoinPolyline polyline;
};

int main(int, char**){
  Loop055 test;
  test.Run();

  return 0;
}
