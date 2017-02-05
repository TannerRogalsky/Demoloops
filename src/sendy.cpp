#include "demoloop.h"
#include <SDL.h>
#include <SDL_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "graphics/shader.h"
#include "graphics/canvas.h"
#include "res_path.h"
#include "cleanup.h"
using namespace std;
using namespace demoloop;

float t = 0;
const float CYCLE_LENGTH = 7;

const static std::string colorShaderCode = R"===(
uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
vec2 rotate(vec2 pos, float rad) {
  return .5 + (pos-.5) * mat2(cos(rad), -sin(rad), sin(rad), cos(rad));
}

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vertpos.xy *= length(vertpos.xy);
  vertpos.x *= dot(vertpos.xy, vertpos.xy) * 0.5;
  // vertpos.xy = rotate(vertpos.xy, vertpos.y);
  // vertpos.xy = rotate(vertpos.xy - length(vertpos.xy), cycle_ratio * DEMOLOOP_M_PI * 2);
  // vertpos.xy = rotate(vertpos.xy - length(vertpos.xy), length(vertpos.xy));
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
#define PI 3.14159265359
#define TWO_PI 6.28318530718

vec4 h2rgb(float c) {
  return  clamp(abs( fract(c + vec4(3,2,1,0)/3.) * 6. - 3.) -1. , 0., 1.);
}

vec4 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return vec4(c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y), 1.0);
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  return Texel(texture, tc) * color * hsv2rgb(vec3(1.0 - tc.x, 0.75, 0.8)) + vec4(pow(1.0 - tc.y, 3.0));
}
#endif
)===";

const static std::string transformShaderCode = R"===(
uniform mediump float cycle_ratio;

#ifdef VERTEX
vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
#define PI 3.14159265359
#define TWO_PI 6.28318530718

vec2 rotate(vec2 pos, float rad) {
  return .5 + (pos-.5) * mat2(cos(rad), -sin(rad), sin(rad), cos(rad));
}

vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  float t = cycle_ratio;
  // tc = tc*-2.+1.;
  tc = rotate(tc, length(tc - 0.5) * 2. * sin(cycle_ratio * TWO_PI));
  return Texel(texture, tc) * color;
}
#endif
)===";

class Loop050 : public Demoloop {
public:
  Loop050() : Demoloop(480, 480, 254, 230, 231), colorShader({colorShaderCode, colorShaderCode}),
              transformShader({transformShaderCode, transformShaderCode}), canvas(width * 2, height * 2) {
    glDisable(GL_DEPTH_TEST);
    {
      TTF_Font *font = loadFont(getResourcePath() + "sendy/sans.ttf", 64);
      SDL_Color White = {255, 255, 255, 255};
      SDL_Surface *textSurface = TTF_RenderText_Blended(font, "sendy", White);
      cleanup(font);

      glGenTextures(1, &text);
      glBindTexture(GL_TEXTURE_2D, text);

      int mode = GL_RGB;
      if(textSurface->format->BytesPerPixel == 4) {
        mode = GL_RGBA;
      }

      textWidth = textSurface->w;
      textHeight = textSurface->h;
      glTexImage2D(GL_TEXTURE_2D, 0, mode, textSurface->w, textSurface->h, 0, mode, GL_UNSIGNED_BYTE, textSurface->pixels);
      cleanup(textSurface);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    GL::TempTransform t1(gl);
    t1.get() = glm::translate(t1.get(), {canvas.getWidth() / 2, canvas.getHeight() / 2, 0});
    t1.get() = glm::scale(t1.get(), {2, 2, 1});

    setCanvas(&canvas);
    colorShader.attach();
    // colorShader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);

    const float interval = DEMOLOOP_M_PI * 2 / NUM_ARMS;
    float phi = 0;
    for (uint32_t i = 0; i < NUM_ARMS; ++i, phi += interval) {
      glm::mat4 m;
      m = glm::rotate(m, phi, {0, 0, 1});
      m = glm::translate(m, {0, 0, 1});
      m = glm::scale(m, {15, 150, 1});
      renderTexture(gl.getDefaultTexture(), m);
    }

    colorShader.detach();
    setCanvas();
  }

  ~Loop050() {
    // cleanup(text);
  }

  void Update(float dt) {
    t += dt;

    const float cycle = fmod(t, CYCLE_LENGTH);
    const float cycle_ratio = cycle / CYCLE_LENGTH;

    // GL::TempTransform t1(gl);
    // t1.get() = glm::scale(t1.get(), {0.5, 0.5, 1});

    setColor(255, 255, 255);
    setBlendMode(BLEND_ALPHA, BLENDALPHA_PREMULTIPLIED);
    transformShader.attach();
    transformShader.sendFloat("cycle_ratio", 1, &cycle_ratio, 1);
    canvas.draw(glm::scale(glm::mat4(), {0.5, 0.5, 1}));
    transformShader.detach();
    setBlendMode(BLEND_ALPHA, BLENDALPHA_MULTIPLY);

    {
      setColor(255, 255, 255, 50);
      glm::mat4 m;
      m = glm::translate(m, {width / 2, height / 2, 0});
      m = glm::rotate(m, sinf(-cycle_ratio * DEMOLOOP_M_PI * 2) * 0.9f, {0, 0, 1});
      m = glm::translate(m, {-width / 2, -height / 2, 0});
      m = glm::scale(m, {0.5, 0.5, 1});
      canvas.draw(m);
    }

    {
      setColor(222, 103, 101);
      renderTexture(text, width * 0.5 - textWidth * 0.5, height * 0.5 - textHeight * 0.5, textWidth, textHeight);
    }
  }

private:
  Shader colorShader;
  Shader transformShader;
  Canvas canvas;
  GLuint text;
  float textWidth, textHeight;
  uint32_t NUM_ARMS = 16;
};

int main(int, char**){
  Loop050 test;
  test.Run();

  return 0;
}
