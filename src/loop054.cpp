#include "demoloop.h"
#include "graphics/3d_primitives.h"
#include "graphics/mesh.h"
#include "helpers.h"
#include "graphics/shader.h"
#include <glm/gtx/rotate_vector.hpp>
using namespace std;
using namespace demoloop;

const uint32_t CYCLE_LENGTH = 10;

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;
varying vec4 frontColor; // color for front face
varying vec4 backColor; // color for back face

#ifdef VERTEX
uniform mat4 v_inv;
attribute vec3 v_normal;

struct lightSource
{
  vec4 position;
  vec4 diffuse;
  vec4 specular;
  float constantAttenuation, linearAttenuation, quadraticAttenuation;
  float spotCutoff, spotExponent;
  vec3 spotDirection;
};
lightSource light0 = lightSource(
  vec4(0.0,  0.0,  4.0, 1.0),
  vec4(1.0,  1.0,  1.0, 1.0),
  vec4(1.0,  0.2,  0.2, 1.0),
  0.0, 0.8, 0.0,
  180.0, 0.0,
  vec3(0.0, 0.0, 0.0)
);
vec4 scene_ambient = vec4(0.2, 0.2, 0.2, 1.0);

struct material
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};
material frontMaterial = material(
  vec4(0.4, 0.4, 0.4, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  5.0
);
material backMaterial = material(
  vec4(0.4, 0.4, 0.4, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  5.0
);

vec4 position(mat4 transform_proj, mat4 m, vec4 v_coord) {
  mat4 mvp = transform_proj * m;
  vec3 normalDirection = normalize(NormalMatrix * v_normal);
  vec3 viewDirection = normalize(vec3(v_inv * vec4(0.0, 0.0, 0.0, 1.0) - m * v_coord));
  vec3 lightDirection;
  float attenuation;

  if (light0.position.w == 0.0) { // directional light
    attenuation = 1.0; // no attenuation
    lightDirection = normalize(vec3(light0.position));
  } else { // point or spot light (or other kind of light)
    vec3 vertexToLightSource = vec3(light0.position - m * v_coord);
    float distance = length(vertexToLightSource);
    lightDirection = normalize(vertexToLightSource);
    attenuation = 1.0 / (light0.constantAttenuation
       + light0.linearAttenuation * distance
       + light0.quadraticAttenuation * distance * distance);

    if (light0.spotCutoff <= 90.0) { // spotlight
      float clampedCosine = max(0.0, dot(-lightDirection, normalize(light0.spotDirection)));
      if (clampedCosine < cos(radians(light0.spotCutoff))) { // outside of spotlight cone
        attenuation = 0.0;
      } else {
        attenuation = attenuation * pow(clampedCosine, light0.spotExponent);
      }
    }
  }

  // Computation of lighting for front faces
  vec3 ambientLighting = vec3(scene_ambient) * vec3(frontMaterial.ambient);

  vec3 diffuseReflection = attenuation
    * vec3(light0.diffuse) * vec3(frontMaterial.diffuse)
    * max(0.0, dot(normalDirection, lightDirection));

  vec3 specularReflection;
  if (dot(normalDirection, lightDirection) < 0.0) {// light source on the wrong side?
    specularReflection = vec3(0.0, 0.0, 0.0); // no specular reflection
  } else { // light source on the right side
    specularReflection = attenuation * vec3(light0.specular) * vec3(frontMaterial.specular)
      * pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), frontMaterial.shininess);
  }
  frontColor = vec4(ambientLighting + diffuseReflection + specularReflection, 1.0);

  // Computation of lighting for back faces (uses negative normalDirection and back material colors)
  vec3 backAmbientLighting = vec3(scene_ambient) * vec3(backMaterial.ambient);

  vec3 backDiffuseReflection = attenuation
    * vec3(light0.diffuse) * vec3(backMaterial.diffuse)
    * max(0.0, dot(-normalDirection, lightDirection));

  vec3 backSpecularReflection;
  if (dot(-normalDirection, lightDirection) < 0.0) { // light source on the wrong side?
    backSpecularReflection = vec3(0.0, 0.0, 0.0); // no specular reflection
  } else { // light source on the right side
    backSpecularReflection = attenuation * vec3(light0.specular) * vec3(backMaterial.specular)
    * pow(max(0.0, dot(reflect(-lightDirection, -normalDirection), viewDirection)), backMaterial.shininess);
  }
  backColor = vec4(backAmbientLighting + backDiffuseReflection + backSpecularReflection, 1.0);

  return mvp * v_coord;
}
#endif

#ifdef PIXEL
highp vec2 hash2( vec2 p ) {
  vec2 a = vec2(127.1,311.7);
  vec2 b = vec2(269.5,183.3);
  highp vec2 c = vec2(dot(p, a),dot(p, b));
  return fract(sin(c)*43758.5453);
}

float voronoi( in vec2 x )
{
  vec2 n = floor(x);
  vec2 f = fract(x);

  //----------------------------------
  // first pass: regular voronoi
  //----------------------------------
  vec2 mg, mr;

  float md = 8.0;
  for( int j=-1; j<=1; j++ )
  for( int i=-1; i<=1; i++ )
  {
    vec2 g = vec2(float(i),float(j));
    vec2 o = hash2( n + g );
    vec2 r = g + o - f;
    float d = dot(r,r);

    if( d<md )
    {
        md = d;
        mr = r;
        mg = g;
    }
  }

  //----------------------------------
  // second pass: distance to borders
  //----------------------------------
  md = 8.0;
  for( int j=-2; j<=2; j++ )
  for( int i=-2; i<=2; i++ )
  {
    vec2 g = mg + vec2(float(i),float(j));
    vec2 o = hash2( n + g );
    vec2 r = g + o - f;

    if( dot(mr-r,mr-r)>0.1 )
    md = min( md, dot( 0.5*(mr+r), normalize(r-mr) ) );
  }

  return md;
}

vec4 effect(vec4 color, Image texture, vec2 st, vec2 screen_coords) {
  float c = voronoi( 8.0*st );

  // isolines
  vec3 col = c*(0.5 + 0.5*sin(64.0*c))*vec3(1.0);
  // borders
  vec3 borderColor = vec3(1.0,0.6,0.0);
  col = mix(borderColor, col, smoothstep( 0.04, 0.07, c ) );

  if (gl_FrontFacing) {
    color = frontColor;
  } else {
    color = backColor;
  }
  return vec4(col, 1.0) * color;
  // return Texel(texture, st) * color;
}
#endif
)===";

glm::vec3 computeNormal(const Vertex &a, const Vertex &b, const Vertex &c) {
  glm::vec3 v1(a.x, a.y, a.z);
  glm::vec3 v2(b.x, b.y, b.z);
  glm::vec3 v3(c.x, c.y, c.z);
  return normalize(cross(v2 - v1, v3 - v1));
}

const uint32_t stacks = 30, slices = 30;
const uint32_t numVertices = (slices + 1) * (stacks + 1);
const uint32_t numIndices = slices * stacks * 6;

class Test4 : public Demoloop {
public:
  Test4() : Demoloop(CYCLE_LENGTH, 25, 25, 25), shader({shaderCode, shaderCode}) {
    glEnable(GL_CULL_FACE);
    // texture = loadTexture("uv_texture.jpg");
    // glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glGenBuffers(1, &normalsBuffer);
  }

  ~Test4() {
    glDeleteBuffers(1, &normalsBuffer);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();
    // const float mod_ratio = powf(sinf(cycle_ratio * DEMOLOOP_M_PI), 2);

    const uint32_t sliceCount = slices + 1;

    uint32_t index = 0;
    for (uint32_t i = 0; i <= stacks; ++i) {
      const float v = static_cast<float>(i) / stacks;

      for (uint32_t j = 0; j <= slices; ++j) {
        const float u = static_cast<float>(j) / slices;

        float s = u * DEMOLOOP_M_PI;
        float t = v * 2 * DEMOLOOP_M_PI;
        s *= 0.82;

        s = s * 2;
        float phi = s / 2 + cycle_ratio * DEMOLOOP_M_PI * 2;
        float a = 0.65, b = 1.0;
        glm::vec2 size(3.75, 3.25);
        size.y *= 1 + sinf(s + cycle_ratio * DEMOLOOP_M_PI * 2) / 5;

        const uint32_t endSegmentLength = 3;
        const int32_t center = slices / 2;
        const uint32_t distanceFromEnd = abs(abs(center - static_cast<int32_t>(j)) - center);
        if (distanceFromEnd < endSegmentLength) {
          // float r = (float)distanceFromEnd / endSegmentLength;
          // a *= (1.0-sqrt(1.0-r))/(r);
          a *= (float)distanceFromEnd / endSegmentLength;
          b *= (float)distanceFromEnd / endSegmentLength;
        }

        float x, y, z;
        x = a * cosf( t ) * cosf( phi ) - b * sinf( t ) * sinf( phi );
        z = a * cosf( t ) * sinf( phi ) + b * sinf( t ) * cosf( phi );
        y = ( size.y + x ) * sinf( s + cycle_ratio * DEMOLOOP_M_PI * 2 );
        x = ( size.x + x ) * cosf( s + cycle_ratio * DEMOLOOP_M_PI * 2 );

        vertices[index] = {
          x, y, z,
          s / 2, t / 2
        };
        normals[index] = glm::vec3(0, 0, 0);
        index++;
      }
    }

    index = 0;
    for (uint32_t i = 0; i < stacks; ++i) {
      for (uint32_t j = 0; j < slices; ++j) {

        const uint32_t a = i * sliceCount + j;
        const uint32_t b = i * sliceCount + j + 1;
        const uint32_t c = ( i + 1 ) * sliceCount + j + 1;
        const uint32_t d = ( i + 1 ) * sliceCount + j;

        // faces one and two
        indices[index++] = a;
        indices[index++] = b;
        indices[index++] = d;
        glm::vec3 faceNormal1 = computeNormal(vertices[a], vertices[b], vertices[d]);
        normals[a] = glm::normalize(normals[a] + faceNormal1);
        normals[b] = glm::normalize(normals[b] + faceNormal1);
        normals[c] = glm::normalize(normals[c] + faceNormal1);

        indices[index++] = b;
        indices[index++] = c;
        indices[index++] = d;
        glm::vec3 faceNormal2 = computeNormal(vertices[b], vertices[c], vertices[d]);
        normals[b] = glm::normalize(normals[b] + faceNormal2);
        normals[c] = glm::normalize(normals[c] + faceNormal2);
        normals[d] = glm::normalize(normals[d] + faceNormal2);
      }
    }

    {
      // const glm::vec3 eye = glm::rotate(glm::vec3(0, 0, 10), static_cast<float>(cycle_ratio * DEMOLOOP_M_PI * 2), glm::vec3(0, 1, 0));
      const glm::vec3 eye = glm::vec3(0, 0, 10);
      const glm::vec3 target = {0, 0, 0};
      const glm::vec3 up = {0, 1, 0};
      glm::mat4 camera = glm::lookAt(eye, target, up);

      GL::TempTransform t1(gl);
      t1.get() = camera;

      GL::TempProjection p1(gl);
      p1.get() = glm::perspective((float)DEMOLOOP_M_PI / 4.0f, (float)width / (float)height, 0.1f, 100.0f);

      glm::mat4 m;
      m = glm::translate(m, {0, -0.5, 0});
      // m = glm::scale(m, {2, 2, 2});
      // m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(0, 1, 0));
      // m = glm::rotate(m, (float)-DEMOLOOP_M_PI / 4, glm::vec3(0, 1, 0));
      // m = glm::rotate(m, cycle_ratio * (float)DEMOLOOP_M_PI * 2, glm::vec3(1, 0, 0));

      shader.attach();

      // gl.bindTexture(texture);

      gl.prepareDraw(m);

      glm::mat4 v_inv = glm::inverse(t1.get());
      shader.sendMatrix("v_inv", 4, &v_inv[0][0], 1);
      // shader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

      gl.bufferVertices(&vertices[0], numVertices);
      glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
      glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
      glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

      uint32_t normalsLocation = shader.getAttribLocation("v_normal");
      glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
      glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), &normals[0].x, GL_DYNAMIC_DRAW);
      glVertexAttribPointer(normalsLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

      gl.bufferIndices(&indices[0], numIndices);

      gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD | (1u << normalsLocation));
      gl.drawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

      shader.detach();

      // mesh.draw(m);
    }

    // renderTexture(texture, 0, 0, 1, height, height);
  }

private:
  GLuint normalsBuffer;
  // GLuint texture;
  Shader shader;
  Vertex vertices[numVertices];
  glm::vec3 normals[numVertices];
  uint32_t indices[numIndices];
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
