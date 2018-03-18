#include "demoloop.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/3d_primitives.h"
#include "graphics/shader.h"
#include "graphics/mesh.h"
#include "math_helpers.h"
#include <array>
using namespace std;
using namespace demoloop;


const uint32_t CYCLE_LENGTH = 10;
const uint32_t arms = 20;
const uint32_t trisPerArm = 20;
const uint32_t numTris = arms * trisPerArm;

glm::vec3 computeNormal(const Triangle &t) {
  glm::vec3 v1(t.a.x, t.a.y, t.a.z);
  glm::vec3 v2(t.b.x, t.b.y, t.b.z);
  glm::vec3 v3(t.c.x, t.c.y, t.c.z);
  return normalize(cross(v2 - v1, v3 - v1));
}

glm::vec3 computeNormal(const Triangle &t1, const Triangle &t2, const Triangle &t3) {
  return normalize(computeNormal(t1) + computeNormal(t2) + computeNormal(t3));
}

glm::vec3 computeNormal(const Vertex &a, const Vertex &b, const Vertex &c) {
  glm::vec3 v1(a.x, a.y, a.z);
  glm::vec3 v2(b.x, b.y, b.z);
  glm::vec3 v3(c.x, c.y, c.z);
  return normalize(cross(v2 - v1, v3 - v1));
}

const float RADIUS = 0.1;
const float interval = DEMOLOOP_M_PI * 2 / 3;
const Triangle triangle = {
  {RADIUS * cosf(interval * 0), RADIUS * sinf(interval * 0), 0},
  {RADIUS * cosf(interval * 1), RADIUS * sinf(interval * 1), 0},
  {RADIUS * cosf(interval * 2), RADIUS * sinf(interval * 2), 0}
};
const Vertex peak = {0, 0, sqrtf(2) * RADIUS};
const Triangle tetrahedron[4] = {
  {triangle.a, triangle.c, triangle.b}, // bottom
  {triangle.a, triangle.b, peak}, // a - b
  {triangle.b, triangle.c, peak}, // b - c
  {triangle.c, triangle.a, peak}, // c - a
};
// const uint32_t numVertices = 4 * 3;
const glm::vec3 tetrahedronNormalsOld[4 * 3] = {
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]), // bottom1
  computeNormal(tetrahedron[0], tetrahedron[2], tetrahedron[3]), // bottom2
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]), // bottom3

  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]), // A1
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]), // A2
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]), // A3

  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[2]), // B1
  computeNormal(tetrahedron[0], tetrahedron[2], tetrahedron[3]), // B2
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]), // B3

  computeNormal(tetrahedron[0], tetrahedron[2], tetrahedron[3]), // C1
  computeNormal(tetrahedron[0], tetrahedron[1], tetrahedron[3]), // C2
  computeNormal(tetrahedron[1], tetrahedron[2], tetrahedron[3]), // C3
};

const array<Vertex, 4> originalVertices = {{
  {1,  1,  1},
  {-1, -1,  1},
  {-1,  1, -1},
  {1, -1, -1}
}};

const array<uint32_t, 12> indices = {{
  2,  1,  0,    0,  3,  2,    1,  3,  0,    2,  3,  1
}};

template<class T>
constexpr T pow(const T base, unsigned const exponent) {
    // (parentheses not required in next line)
    return (exponent == 0)     ? 1 :
           (exponent % 2 == 0) ? pow(base, exponent/2)*pow(base, exponent/2) :
           base * pow(base, (exponent-1)/2) * pow(base, (exponent-1)/2);
}

template<uint32_t detail>
void subdivideFace(Vertex *out, uint32_t &index, const Vertex &a, const Vertex &b, const Vertex &c) {
  const uint32_t cols = pow(2, detail);
  array<array<Vertex, cols + 1>, cols + 1> v;

  for (uint32_t i = 0; i <= cols; ++i) {
    const uint32_t rows = cols - i;

    Vertex aj = mix(a, c, static_cast<float>(i) / cols);
    Vertex bj = mix(b, c, static_cast<float>(i) / cols);

    for (uint32_t j = 0; j <= rows; ++j) {
      if (j == 0 && i == cols) {
        v[i][j] = aj;
      } else {
        v[i][j] = mix(aj, bj, static_cast<float>(j) / rows);
      }
    }
  }

  for (uint32_t i = 0; i < cols; ++i) {
    for (uint32_t j = 0; j < 2 * (cols - i) - 1; ++j) {
      const uint32_t k = floor(static_cast<float>(j) / 2.0);

      if (j % 2 == 0) {
        out[index++] = v[ i ][ k + 1 ];
        out[index++] = v[ i + 1 ][ k ];
        out[index++] = v[ i ][ k ];
      } else {
        out[index++] = v[ i ][ k + 1 ];
        out[index++] = v[ i + 1 ][ k + 1 ];
        out[index++] = v[ i + 1 ][ k ];
      }
    }
  }
}

template<
  uint32_t DETAIL = 0,
  size_t NUM_VERTS,
  size_t NUM_INDICES,
  typename = typename std::enable_if<DETAIL >= 0>::type>
array<Vertex, NUM_INDICES * pow<uint32_t>(4, DETAIL)> subdivide(const array<Vertex, NUM_VERTS> &in, const array<uint32_t, NUM_INDICES> &indices) {
  array<Vertex, NUM_INDICES * pow<uint32_t>(4, DETAIL)> out;
  uint32_t index = 0;

  for (uint32_t i = 0; i < indices.size(); i+=3) {
    const Vertex &a = in[indices[i + 0]];
    const Vertex &b = in[indices[i + 1]];
    const Vertex &c = in[indices[i + 2]];

    subdivideFace<DETAIL>(out.data(), index, a, b, c);
  }

  return out;
}

const auto vertices = subdivide<0>(originalVertices, indices);

template<size_t N>
array<glm::vec3, N> getNormals(const array<Vertex, N> &verts) {
  array<glm::vec3, N> out;
  for (uint32_t i = 0; i < verts.size(); ++i) {
    float x = verts[i].x, y = verts[i].y, z = verts[i].z;

    float n = 1.0 / sqrtf(x * x + y * y + z * z);

    out[i].x = x * n;
    out[i].y = y * n;
    out[i].z = z * n;
  }

  return out;
}

const auto tetrahedronNormals = getNormals(vertices);

const static std::string shaderCode = R"===(
varying vec4 vpos;
varying vec3 vNorm;
varying vec4 vColor;
uniform mat4 v_inv;

varying vec4 varyingPosition;  // position of the vertex (and fragment) in world space
varying vec3 varyingNormalDirection;  // surface normal vector in world space

#ifdef VERTEX
attribute vec3 v_normal;

vec4 position(mat4 transform_proj, mat4 model, vec4 v_coord) {
  varyingPosition = model * v_coord;
  mat4 mvp = transform_proj * model;
  varyingNormalDirection = normalize(NormalMatrix * v_normal);
  return mvp * v_coord;
}
#endif

#ifdef PIXEL
struct lightSource {
  vec4 position;
  vec4 diffuse;
  vec4 specular;
  float constantAttenuation, linearAttenuation, quadraticAttenuation;
  float spotCutoff, spotExponent;
  vec3 spotDirection;
};
extern lightSource light0 = lightSource(
  vec4(0.0,  -1.0,  0.0, 1.0),
  vec4(1.0,  1.0,  1.0, 1.0),
  vec4(1.0,  1.0,  1.0, 1.0),
  0.0, 1.0, 0.0,
  180.0, 0.0,
  vec3(0.0, 0.0, 0.0)
);
vec4 scene_ambient = vec4(0.0, 0.0, 0.0, 1.0);

struct material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};
material frontMaterial = material(
  vec4(1.0, 1.0, 1.0, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  5.0
);

vec4 effect(vec4 color, Image texture, vec2 texture_coords, vec2 screen_coords) {
  vec3 normalDirection = normalize(varyingNormalDirection);
  vec3 viewDirection = normalize(vec3(v_inv * vec4(0.0, 0.0, 0.0, 1.0) - varyingPosition));
  vec3 lightDirection;
  float attenuation;

  if (0.0 == light0.position.w) // directional light?
    {
      attenuation = 1.0; // no attenuation
      lightDirection = normalize(vec3(light0.position));
    }
  else // point light or spotlight (or other kind of light)
    {
      vec3 positionToLightSource = vec3(light0.position - varyingPosition);
      float distance = length(positionToLightSource);
      lightDirection = normalize(positionToLightSource);
      attenuation = 1.0 / (light0.constantAttenuation
                           + light0.linearAttenuation * distance
                           + light0.quadraticAttenuation * distance * distance);

      if (light0.spotCutoff <= 90.0) // spotlight?
  {
    float clampedCosine = max(0.0, dot(-lightDirection, light0.spotDirection));
    if (clampedCosine < cos(radians(light0.spotCutoff))) // outside of spotlight cone?
      {
        attenuation = 0.0;
      }
    else
      {
        attenuation = attenuation * pow(clampedCosine, light0.spotExponent);
      }
  }
    }

  vec3 ambientLighting = vec3(scene_ambient) * vec3(frontMaterial.ambient);

  vec3 diffuseReflection = attenuation
    * vec3(light0.diffuse) * vec3(frontMaterial.diffuse)
    * max(0.0, dot(normalDirection, lightDirection));

  vec3 specularReflection;
  if (dot(normalDirection, lightDirection) < 0.0) // light source on the wrong side?
    {
      specularReflection = vec3(0.0, 0.0, 0.0); // no specular reflection
    }
  else // light source on the right side
    {
      specularReflection = attenuation * vec3(light0.specular) * vec3(frontMaterial.specular)
  * pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), frontMaterial.shininess);
    }

  return vec4(ambientLighting + diffuseReflection + specularReflection, 1.0);
}
#endif
)===";

uint32_t bufferMatrixAttribute(const GLint location, const GLuint buffer, const float *data, const uint32_t num, GLenum usage) {
  uint32_t enabledAttribs = 0;
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, num * sizeof(float) * 4 * 4, data, usage);

  for (int i = 0; i < 4; ++i) {
    enabledAttribs |= 1u << (uint32_t)(location + i);
    glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*)(sizeof(float) * (4 * i)));
    glVertexAttribDivisor(location + i, 1);
  }

  return enabledAttribs;
}

const uint32_t stacks = 30, slices = 30;
const uint32_t numVertices = (slices + 1) * (stacks + 1);
const uint32_t numIndices = slices * stacks * 6;

class Loop047 : public Demoloop {
public:
  Loop047() : Demoloop(CYCLE_LENGTH, 150, 150, 150), shader({shaderCode, shaderCode}), mesh(plane(5, 5, stacks, slices)), sphereMesh(sphere(0.1, stacks, slices)) {
    glm::mat4 perspective = glm::perspective(static_cast<float>(DEMOLOOP_M_PI) / 4.0f, (float)width / (float)height, 0.1f, 100.0f);
    gl.getProjection() = perspective;

    glGenBuffers(1, &normalsBuffer);

    // gl.bufferVertices((Vertex *)&tetrahedron, numVertices, GL_STATIC_DRAW);
    // glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    // glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    fill(begin(planeNormals), end(planeNormals), glm::vec3(0, 0, 0));
    uint32_t index = 0, sliceCount = slices + 1;
    for (uint32_t i = 0; i < stacks; ++i) {
      for (uint32_t j = 0; j < slices; ++j) {

        const uint32_t a = i * sliceCount + j;
        const uint32_t b = i * sliceCount + j + 1;
        const uint32_t c = ( i + 1 ) * sliceCount + j + 1;
        const uint32_t d = ( i + 1 ) * sliceCount + j;

        // faces one and two
        mesh.mIndices[index++] = a;
        mesh.mIndices[index++] = b;
        mesh.mIndices[index++] = d;
        glm::vec3 faceNormal1 = computeNormal(mesh.mVertices[a], mesh.mVertices[b], mesh.mVertices[d]);
        planeNormals[a] = glm::normalize(planeNormals[a] + faceNormal1);
        planeNormals[b] = glm::normalize(planeNormals[b] + faceNormal1);
        planeNormals[c] = glm::normalize(planeNormals[c] + faceNormal1);

        mesh.mIndices[index++] = b;
        mesh.mIndices[index++] = c;
        mesh.mIndices[index++] = d;
        glm::vec3 faceNormal2 = computeNormal(mesh.mVertices[b], mesh.mVertices[c], mesh.mVertices[d]);
        planeNormals[b] = glm::normalize(planeNormals[b] + faceNormal2);
        planeNormals[c] = glm::normalize(planeNormals[c] + faceNormal2);
        planeNormals[d] = glm::normalize(planeNormals[d] + faceNormal2);
      }
    }

    normalsLocation = shader.getAttribLocation("v_normal");

    const glm::vec3 eye = glm::vec3(0, 0, -4);
    const glm::vec3 up = glm::vec3(0, 1, 0);
    const glm::vec3 target = glm::vec3(0, 0, 0);
    camera = glm::lookAt(eye, target, up);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    // const glm::vec3 eye = glm::rotate(glm::vec3(0, 1, 4), cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
    // const glm::vec3 eye = glm::vec3(0, 1, 4);
    // const glm::vec3 up = glm::vec3(0, 1, 0);
    // const glm::vec3 target = glm::vec3(0, 0, 0);
    // glm::mat4 camera = glm::lookAt(eye, target, up);

    if (isMouseDown(SDL_BUTTON_LEFT)) {
      float rotateSpeed = 1;
      camera = glm::rotate(camera, 2 * static_cast<float>(DEMOLOOP_M_PI) * getMouseDeltaX() / width * rotateSpeed, glm::vec3(0, 1, 0));
      camera = glm::rotate(camera, 2 * static_cast<float>(DEMOLOOP_M_PI) * getMouseDeltaY() / height * rotateSpeed, glm::vec3(1, 0, 0));
    }

    GL::TempTransform t1(gl);
    t1.get() = camera;

    glm::mat4 m;
    // m = glm::translate(m, {sinf(cycle_ratio * DEMOLOOP_M_PI * 2), 0, 0});
    m = glm::rotate(m, (float)DEMOLOOP_M_PI, {1, 0, 0});
    m = glm::translate(m, {0, 0, -0.5});
    // m = glm::rotate(m, (float)DEMOLOOP_M_PI * 2 * cycle_ratio, {0, 0, 1});

    glm::vec4 lightPosition(0, 0, -1, 1);
    setColor(255, 99, 71);
    sphereMesh.draw(glm::translate(glm::mat4(), {lightPosition.x, lightPosition.y, lightPosition.z}));

    shader.attach();
    shader.sendFloat("light0.position", 4, &lightPosition.x, 1);

    {
      RGB c = hsl2rgb(cycle_ratio, 1, 0.5);
      glm::vec4 color = glm::vec4(c.r / 255.0, c.g / 255.0, c.b / 255.0, 1.0);
      shader.sendFloat("light0.diffuse", 4, &color.x, 1);
    }

    // gl.bindTexture(texture);
    setColor(255, 255, 255);
    gl.prepareDraw(m);

    glm::mat4 v_inv = glm::inverse(t1.get());
    shader.sendMatrix("v_inv", 4, &v_inv[0][0], 1);
    // shader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    gl.bufferVertices(mesh.mVertices.data(), mesh.mVertices.size());
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &planeNormals[0].x, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    gl.bufferIndices(mesh.mIndices.data(), mesh.mIndices.size());

    gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD | (1u << normalsLocation));
    gl.drawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);



    for (uint32_t i = 0; i < 1; ++i) {
      // const float i_cycle_ratio = fmod((float)i / 5 + cycle_ratio, 1);
      // setColor(hsl2rgb(i_cycle_ratio, 1, 0.5));

      m = glm::mat4();
      m = glm::scale(m, {0.25, 0.25, 0.25});
      // m = glm::rotate(m, (float)DEMOLOOP_M_PI * i_cycle_ratio * 2, {0, 1, 0});

      m = glm::rotate(m, (float)DEMOLOOP_M_PI * cycle_ratio * 2, {1, 0, 0});
      // m = glm::translate(m, {0, 0, sqrtf(2) * -RADIUS / 2});


      // m = glm::translate(m, {sinf(i_cycle_ratio * DEMOLOOP_M_PI * 2) * 2, 0.5, 0});
      m = glm::translate(m, {0, 0, -1});
      // m = glm::rotate(m, (float)DEMOLOOP_M_PI * cycle_ratio * 4, {0, 0, 1});
      gl.prepareDraw(m);

      // gl.bufferVertices((Vertex *)tetrahedron, 4 * 3);
      gl.bufferVertices(vertices.data(), vertices.size());
      glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
      glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
      glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

      glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
      glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &tetrahedronNormals[0].x, GL_DYNAMIC_DRAW);
      glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

      gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD | (1u << normalsLocation));
      gl.drawArrays(GL_TRIANGLES, 0, vertices.size());

      // setColor(0, 0, 0);
      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    shader.detach();

    // gl.triangles(&triangle, 1, glm::scale(glm::mat4(), {4, 4, 4}));
  }

private:
  uint32_t normalsLocation;
  glm::mat4 matrices[numTris];
  glm::vec4 colors[numTris];
  glm::vec3 planeNormals[numIndices];
  Shader shader;
  Mesh mesh;
  Mesh sphereMesh;
  GLuint normalsBuffer;
  glm::mat4 camera;
};

int main(int, char**){
  Loop047 test;
  test.Run();

  return 0;
}
