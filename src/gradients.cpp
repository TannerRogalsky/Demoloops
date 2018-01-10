#include "demoloop.h"
#include "math_helpers.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/2d_primitives.h"
#include "graphics/shader.h"
#include "graphics/image.h"
#include <array>
#include <vector>
using namespace std;
using namespace demoloop;

const Vertex quad[6] = {
  {0, 0, 0, 0, 0}, {0, 1, 0, 0, 1}, {1, 0, 0, 1, 0},
  {1, 1, 0, 1, 1}, {0, 1, 0, 0, 1}, {1, 0, 0, 1, 0}
};

const static std::string gradientCode = R"===(
#define MAX_CONTROL_POINTS 15
uniform mediump float linearGradient;
uniform mediump int controlPointCount;
uniform mediump float controlPointRatios[MAX_CONTROL_POINTS];
uniform mediump vec4 controlPointColors[MAX_CONTROL_POINTS];

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float r = mix(length(tc * 2.0 - 1.0), tc.x, linearGradient);

  float controlPoint = smoothstep(controlPointRatios[0], controlPointRatios[1], r);
  vec4 mixed = mix(controlPointColors[0], controlPointColors[1], controlPoint);
  for (int i = 1; i < controlPointCount - 1; ++i) {
    controlPoint = smoothstep(controlPointRatios[i], controlPointRatios[i + 1], r);
    mixed = mix(mixed, controlPointColors[i + 1], controlPoint);
  }
  return mixed * color;
}
#endif
)===";

const static std::string gradientTextureCode = R"===(
uniform mediump float linearGradient;

varying vec2 vGradientCoords;

#ifdef VERTEX
attribute vec2 gradientCoords;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vGradientCoords = gradientCoords;

  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float r = mix(length(tc * 2.0 - 1.0), vGradientCoords.x, linearGradient);
  vec2 gradientIndex = vec2(r, vGradientCoords.y);
  vec4 mixed = Texel(texture, gradientIndex);
  return mixed * color;
}
#endif
)===";

class Gradients : public Demoloop {
public:
  Gradients() : Demoloop(720 * 1.5, 720 * 1.5, 0, 0, 0),
                gradient({gradientCode, gradientCode}),
                gradientTexture({gradientTextureCode, gradientTextureCode}),
                gradientWidth(width / 2), gradientHeight(height / 2) {
    glDisable(GL_DEPTH_TEST);

    // http://www.color-hex.com/color-palette/52745
    array<glm::vec4, 5> colors = {{
      glm::vec4(239 / 255.0,74 / 255.0,167 / 255.0, 1.0),
      glm::vec4(210 / 255.0,27 / 255.0,41 / 255.0, 1.0),
      glm::vec4(36 / 255.0,156 / 255.0,203 / 255.0, 1.0),
      glm::vec4(185 / 255.0,185 / 255.0,60 / 255.0, 1.0),
      glm::vec4(250 / 255.0,208 / 255.0,73 / 255.0, 1.0),
    }};
    array<float, colors.size()> ratios;
    generate(ratios.begin(), ratios.end(), [n = 0.f, t = colors.size()]() mutable { return n++ / t; });
    const GLint count = colors.size();
    gradient.sendInt("controlPointCount", 1, &count, 1);
    gradient.sendFloat("controlPointColors", 4, &colors[0].x, colors.size());
    gradient.sendFloat("controlPointRatios", 1, &ratios[0], ratios.size());

    {
      const uint32_t pxDimensions = 64;
      vector<uint8_t> pixels(pxDimensions * pxDimensions * 4);
      const uint32_t y = pxDimensions - 1;
      // for (uint32_t y = 0; y < pxDimensions; ++y) {
        for (uint32_t x = 0; x < pxDimensions; ++x) {
          const uint32_t index = 4 * (x + (y * pxDimensions));

          float xRatio = static_cast<float>(x) / pxDimensions;

          size_t indexLow = 0;
          size_t indexHigh = ratios.size() - 1;
          // get the stops that bound our current ratio
          for (size_t i = 0; i < ratios.size() - 1; ++i) {
              if (ratios[i] <= xRatio && ratios[i + 1] > xRatio) {
                  indexLow = i;
                  indexHigh = i + 1;
                  break;
              }
          }
          // normalize xRatio to our ratio bounds
          float colorRatio = (xRatio - ratios[indexLow]) / (ratios[indexHigh] - ratios[indexLow]);
          auto colorLow = colors[indexLow];
          auto colorHigh = colors[indexHigh];
          pixels[index + 0] = static_cast<uint8_t>(0xFF * (std::max(0.f, mix(colorLow.r, colorHigh.r, colorRatio))));
          pixels[index + 1] = static_cast<uint8_t>(0xFF * (std::max(0.f, mix(colorLow.g, colorHigh.g, colorRatio))));
          pixels[index + 2] = static_cast<uint8_t>(0xFF * (std::max(0.f, mix(colorLow.b, colorHigh.b, colorRatio))));
          pixels[index + 3] = static_cast<uint8_t>(0xFF * (std::max(0.f, mix(colorLow.a, colorHigh.a, colorRatio))));
        }
      // }
      gradientImage = make_unique<Image>(std::move(pixels), pxDimensions, pxDimensions);

      glGenBuffers(1, &gradientCoordsBuffer);
    }
  }

  ~Gradients() {
    glDeleteBuffers(1, &gradientCoordsBuffer);
  }

  void Update(float /*dt*/) {
    gradient.attach();

    float linearGradientToggle = 1;
    gradient.sendFloat("linearGradient", 1, &linearGradientToggle, 1);
    gl.triangles(quad, 6, glm::scale(glm::mat4(), {gradientWidth, gradientHeight, 1}));

    linearGradientToggle = 0;
    gradient.sendFloat("linearGradient", 1, &linearGradientToggle, 1);
    gl.triangles(quad, 6, glm::scale(glm::translate(glm::mat4(), {gradientWidth, 0.f, 0.f}), {gradientWidth, gradientHeight, 1}));

    gradient.detach();


    {
      glm::mat4 transform = glm::translate(glm::mat4(), {0.f, gradientHeight, 0.f});
      // gradientImage->draw(transform);
      gl.bindTexture(*(GLuint *) gradientImage->getHandle());

      gradientTexture.attach();

      // const uint32_t padding = 2;
      // const float gu = static_cast<float>(gradientWidth - padding) / gradientWidth;
      // const float gv = static_cast<float>(gradientHeight - padding) / gradientHeight;
      // array<glm::vec2, 6> gradientCoords = {{
      //   {0, 0}, {0, gv}, {gu, 0},
      //   {gu, gv}, {0, gv}, {gu, 0},
      // }};
      array<glm::vec2, 6> gradientCoords = {{
        {0, 1}, {0, 1}, {1, 1},
        {1, 1}, {0, 1}, {1, 1},
      }};

      uint32_t gradientCoordsLocation = gradientTexture.getAttribLocation("gradientCoords");
      glBindBuffer(GL_ARRAY_BUFFER, gradientCoordsBuffer);
      glBufferData(GL_ARRAY_BUFFER, gradientCoords.size() * sizeof(glm::vec2), &gradientCoords[0].x, GL_DYNAMIC_DRAW);
      glVertexAttribPointer(gradientCoordsLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

      gl.bufferVertices(quad, 6, GL_DYNAMIC_DRAW);

      gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD | (1u << gradientCoordsLocation));
      glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
      glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));

      linearGradientToggle = 1;
      gradientTexture.sendFloat("linearGradient", 1, &linearGradientToggle, 1);
      gl.prepareDraw(glm::scale(transform, {gradientWidth, gradientHeight, 1}));
      gl.drawArrays(GL_TRIANGLES, 0, 6);

      linearGradientToggle = 0;
      gradientTexture.sendFloat("linearGradient", 1, &linearGradientToggle, 1);
      gl.prepareDraw(glm::scale(glm::translate(glm::mat4(), {gradientWidth, gradientHeight, 0.f}), {gradientWidth, gradientHeight, 1}));
      gl.drawArrays(GL_TRIANGLES, 0, 6);

      gradientTexture.detach();
    }
  }

private:
  Shader gradient, gradientTexture;
  uint32_t gradientWidth, gradientHeight;
  unique_ptr<Image> gradientImage;
  GLuint gradientCoordsBuffer;
};

int main(int, char**){
  Gradients test;
  test.Run();

  return 0;
}
