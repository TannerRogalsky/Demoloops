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

const static std::string bgShaderCode = R"===(
#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
extern float cycle_ratio;

float random (in vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
  // return fract(sin(mod(dot(st, st), 7.0 / 2.0)) * 43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    // u = mix(f, u, pow(sin(cycle_ratio * DEMOLOOP_M_PI * 2.0), 2.0));

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 10
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitud = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitud * noise(st);
        st *= 2.;
        amplitud *= .5;
    }
    return value;
}

float pattern( in vec2 p ) {
  vec2 q = vec2( fbm( p + vec2(0.0,0.0) ),
                 fbm( p + vec2(5.2,1.3) ) );

  vec2 r = vec2( fbm( p + 4.0*q + vec2(1.7,9.2) ),
                 fbm( p + 4.0*q + vec2(8.3,2.8) ) );

  return fbm( p + 4.0*r );
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  vec2 offset = vec2(cycle_ratio * 0.1, cycle_ratio / 4.0);
  float t = (1.0 - pow(screen_coords.y / demoloop_ScreenSize.y, 2.0)) * pattern(tc + offset);
  // t *= pow(sin(screen_coords.x / demoloop_ScreenSize.x * 3.14), 2.0) * 0.25 + 0.75;
  // t = pattern(tc + cycle_ratio);
  t *= 1.75;
  return color * vec4(t, t, t, 1.0);
}
#endif
)===";

const static std::string lineShaderCode = R"===(
#ifdef VERTEX
extern float elapsed;
float random (in float t) {
  return fract(t * 43758.5453123);
}

float random (in vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vec4 p = transform_proj * model * vertpos;
  float t = pow(sin(p.y * 3.14 + elapsed), 2.0) * pow(random(elapsed), 2.0) * 0.2;
  if (t > 0.185) {
    p.w += t;
  }
  return p;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float f = smoothstep(0.0, 0.4, tc.x);
  f *= smoothstep(0.0, 0.4, 1.0 - tc.x);
  // return vec4(vec3(f), 1.0 - f);
  return vec4(color.rgb, f);
}
#endif
)===";

class Loop055 : public Demoloop {
public:
  Loop055() : Demoloop(CYCLE_LENGTH, 720, 720, 0, 0, 0),
              bgShader({bgShaderCode, bgShaderCode}),
              lineShader({lineShaderCode, lineShaderCode}) {
    // glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    // gl.getProjection() = perspective;
    glDisable(GL_DEPTH_TEST);

    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});
    gl.getTransform() = glm::scale(gl.getTransform(), {1.125, 1.0, 1});

    glLineWidth(3.0);
  }

  ~Loop055() {
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    const float halfwidth = 3;
    const float pixel_size = 1;
    const bool draw_overdraw = false;

    bgShader.attach();
    const float t = getTime();
    bgShader.sendFloat("cycle_ratio", 1, &t, 1);
    setColor(20, 35, 40);
    renderTexture(gl.getDefaultTexture(), -width / 2, -height / 2, width, height);
    bgShader.detach();

    setColor(206, 3, 0);
    lineShader.attach();
    lineShader.sendFloat("elapsed", 1, &cycle_ratio, 1);
    {
      // circleShader.attach();
      // circleShader.sendFloat("elapsed", 1, &t, 1);
      glm::mat4 transform;
      // transform = glm::translate(transform, {sinf(cycle_ratio * DEMOLOOP_M_PI * 2.0) * width * 0.05, 0, 0});
      lineLoop(hexagon, polygonVertices<50>(width * 0.01), halfwidth, pixel_size, draw_overdraw, transform);
      lineLoop(hexagon, polygonVertices<50>(width * 0.05), halfwidth, pixel_size, draw_overdraw, transform);
      // circleShader.detach();
    }

    {
      glm::mat4 transform;
      transform = glm::rotate(transform, rotationOffset(3), {0, 0, 1});
      lineLoop(hexagon, polygonVertices<3>(width * 0.2), halfwidth, pixel_size, draw_overdraw, transform);
      transform = glm::rotate(transform, (float)DEMOLOOP_M_PI, {0, 0, 1});
      lineLoop(hexagon, polygonVertices<3>(width * 0.2), halfwidth, pixel_size, draw_overdraw, transform);
    }

    {
      glm::mat4 transform;
      transform = glm::rotate(transform, (float)DEMOLOOP_M_PI / 6, {0, 0, 1});
      auto inner = polygonVertices<6>(width * 0.2);
      auto outer = polygonVertices<6>(width * 0.3);

      array<Vertex, 2> lineVerts;
      glm::vec4 temp(0);
      for (int32_t i = 0; i < 6; ++i) {
        memcpy(&temp, &inner[(i + 1) % 6], sizeof(glm::vec3));
        temp = temp * transform;
        memcpy(&lineVerts[0], &temp, sizeof(glm::vec3));
        lineVerts[1] = outer[i];
        line(hexagon, lineVerts, halfwidth, pixel_size, draw_overdraw);
        lineVerts[1] = outer[(i + 1) % 6];
        line(hexagon, lineVerts, halfwidth, pixel_size, draw_overdraw);
      }
    }

    lineLoop(hexagon, polygonVertices<6>(width * 0.3), halfwidth, pixel_size, draw_overdraw);

    {
      const float w = width * 0.3;
      const float h = width * 0.2;
      // const float h = width * 0.2 * (1.0 - powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2.0) * 0.5);
      line(hexagon, spline<30>({-w, 0}, {0, h}, {w, 0}), halfwidth, pixel_size, draw_overdraw);
      line(hexagon, spline<30>({-w, 0}, {0, h * 0.9}, {w, 0}), halfwidth, pixel_size, draw_overdraw);
      line(hexagon, spline<30>({-w, 0}, {0, -h}, {w, 0}), halfwidth, pixel_size, draw_overdraw);
      line(hexagon, spline<30>({-w, 0}, {0, -h * 0.9}, {w, 0}), halfwidth, pixel_size, draw_overdraw);
      // lineLoop(hexagon, polygonVertices<6>(width * 0.4), halfwidth, pixel_size, draw_overdraw);
    }
    lineShader.detach();
  }

private:
  Shader bgShader, lineShader;
  MiterJoinPolyline hexagon;
};

int main(int, char**){
  Loop055 test;
  test.Run();

  return 0;
}
