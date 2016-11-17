#include <iostream>
#include <array>
#include <cmath>
#include <vector>
#include <numeric>
#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/2d_primitives.h"
#include "graphics/mesh.h"
#include "graphics/canvas.h"
#include "helpers.h"
#include "hsl.h"
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 10;

const static std::string shaderCode = R"===(
#define PI 3.141592653589793238462643383279

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords) {
  // vec4 tc = Texel(texture, texture_coords);
  // float albedo = dot(tc.rgb, vec3(0.3, 0.59, 0.11));
  // tc.rgb = vec3(albedo, albedo, albedo);
  float albedo = pow(sin(texture_coords.y * 3.1459), 3);
  vec4 c = vec4(albedo, albedo, albedo, 1);
  return mix(c, vec4(1, 0, 0, 1), pow(sin(texture_coords.x * 3.1459), 6));
}
#endif
)===";

Vertex sphereVertex(const float r, const float theta, const float phi) {
  const float x = r * sinf(theta) * cosf(phi);
  const float y = r * cosf(theta);
  const float z = r * sinf(theta) * sinf(phi);

  const float u = atan2(z, -x) / (DEMOLOOP_M_PI * 2) + 0.5;
  const float v = asin(-y) / DEMOLOOP_M_PI + 0.5;

  return Vertex(x, y, z, u, v, 255, 255, 255, 255);
}

Mesh *uvSphere(const float r, const uint32_t stacks = 30, const uint32_t slices = 30) {
  vector<Vertex> vertices;

  float vInterval = DEMOLOOP_M_PI / stacks;
  float hInterval = DEMOLOOP_M_PI * 2 / (slices - 1);

  for (uint32_t t = 0; t < stacks; ++t) {
    const float theta1 = t * vInterval;
    const float theta2 = (t + 1) * vInterval;

    for (uint32_t p = 0; p < slices; ++p) {
      const float phi1 = p * hInterval;
      const float phi2 = (p + 1) * hInterval;

      // cout << atan2(cosf(theta2), -(sinf(theta2) * cosf(phi2))) / (DEMOLOOP_M_PI * 2) + 0.5f << endl;

      Vertex v1 = sphereVertex(r, theta1, phi1);
      Vertex v2 = sphereVertex(r, theta1, phi2);
      Vertex v3 = sphereVertex(r, theta2, phi2);
      Vertex v4 = sphereVertex(r, theta2, phi1);

      if( t == 0 ) { // top cap
        // mesh->addTri( vertex1, vertex3, vertex4 ) ; //t1p1, t2p2, t2p1
        vertices.push_back(v1);
        vertices.push_back(v3);
        vertices.push_back(v4);
      } else if( t + 1 == stacks ) {//end cap
        // mesh->addTri( vertex3, vertex1, vertex2 ) ; //t2p2, t1p1, t1p2
        vertices.push_back(v3);
        vertices.push_back(v1);
        vertices.push_back(v2);
      } else {
        // body, facing OUT:
        // mesh->addTri( vertex1, vertex2, vertex4 ) ;
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v4);
        // mesh->addTri( vertex2, vertex3, vertex4 ) ;
        vertices.push_back(v2);
        vertices.push_back(v3);
        vertices.push_back(v4);
      }
    }
  }

  vector<uint32_t> indices(vertices.size());
  iota(indices.begin(), indices.end(), 0);
  return new Mesh(vertices, indices);
}

class Test4 : public Demoloop {
public:
  Test4() : Demoloop(1280, 640, 150, 150, 150), mesh(*uvSphere(1)), canvas(next_p2(200), next_p2(200)), shader({shaderCode, shaderCode}) {
    // glEnable(GL_CULL_FACE);
    texture = loadTexture("uv_texture.jpg");

    {
      setCanvas(&canvas);
      shader.attach();
      // setColor(255, 0, 0);
      // rectangle(gl, 0, 0, 100, 100);
      renderTexture(gl.getDefaultTexture(), 0, 0, canvas.getWidth(), canvas.getHeight());

      shader.detach();
      setCanvas();
    }

    mesh.setTexture(&canvas);
  }

  ~Test4() {
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    {
      // const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 4), static_cast<float>(-cycle_ratio * DEMOLOOP_M_PI * 2), glm::vec3(0, 1, 0));
      const glm::vec3 eye = glm::vec3(0, 0, 4);
      const glm::vec3 target = {0, 0, 0};
      const glm::vec3 up = {0, 1, 0};
      glm::mat4 camera = glm::lookAt(eye, target, up);

      GL::TempTransform t1(gl);
      t1.get() = camera;

      GL::TempProjection p1(gl);
      p1.get() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

      glm::mat4 m;
      m = glm::translate(m, {1, 0, 0});
      m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
      // m = glm::rotate(m, (float)-DEMOLOOP_M_PI / 2, glm::vec3(0, 1, 0));
      m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(1, 0, 0));

      t1.get() *= m;
      gl.bindTexture(texture);
      gl.triangles(mesh.mVertices.data(), mesh.mVertices.size());

      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      // setColor(0, 0, 0);
      // gl.bindTexture(gl.getDefaultTexture());
      // gl.triangles(mesh.mVertices.data(), mesh.mVertices.size());
      // setColor(255, 255, 255);
      // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      // mesh.draw(m);
    }

    // shader.attach();
    // canvas.draw();
    renderTexture(texture, 0, 0, 1, height, height);
    // shader.detach();
  }

private:
  Mesh mesh;
  Canvas canvas;
  Shader shader;
  GLuint texture;
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
