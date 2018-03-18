#include "demoloop.h"
#include "helpers.h"
#include "res_path.h"
#include "cleanup.h"
#include "graphics/shader.h"
#include <SDL_ttf.h>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace demoloop;

const float CYCLE_LENGTH = 4;

const static std::string shaderCode = R"===(
uniform mediump float cycle_ratio;

#define DEMOLOOP_M_PI 3.1459

#ifdef VERTEX
uniform sampler2D _tex0_;

vec4 position(mat4 transform_proj, mat4 model, vec4 vertpos) {
  vec4 tex = texture2D(_tex0_, VertexTexCoord.st);
  // vec4 tex = texture2D(_tex0_, fract(VertexTexCoord.st + cycle_ratio + 0.12));
  vec4 tex2 = texture2D(_tex0_, fract(tex.rb + cycle_ratio));

  vertpos.xy += (tex2.rg - vec2(.5)) * vec2(600.0, 1000.0) * pow(sin(cycle_ratio * DEMOLOOP_M_PI), 2.0);

  return transform_proj * model * vertpos;
}
#endif

#ifdef PIXEL
vec4 effect(vec4 color, Image texture, vec2 tc, vec2 screen_coords) {
  // return texture2D(texture, fract(tc + cycle_ratio));
  return texture2D(texture, tc);
}
#endif
)===";

class Test4 : public Demoloop {
public:
  Test4() : Demoloop(CYCLE_LENGTH, 720, 720, 0, 0, 0), shader({shaderCode, shaderCode}) {
    glDisable(GL_DEPTH_TEST);
    gl.getTransform() = glm::translate(gl.getTransform(), {width / 2, height / 2, 0});

    auto font = unique_sdl<TTF_Font>(loadFont(getResourcePath() + "Helvetica-Regular.ttf", height / 6));

    const SDL_Color white = {255, 255, 255, 255};
    auto surf = unique_sdl<SDL_Surface>(TTF_RenderText_Blended(font.get(), "VALENTINES", white));

    points.reserve(surf->h * surf->pitch);

    uint32_t *pixels = static_cast<uint32_t *>(surf->pixels);
    for (int32_t y = 0; y < surf->h; ++y) {
      for (int32_t x = 0; x < surf->w; ++x) {
        int32_t i = (x + y * surf->w);
        uint8_t a = (pixels[i] & surf->format->Amask) >> 24;
        if (a) {
          const float u = static_cast<float>(x) / static_cast<float>(surf->w);
          const float v = static_cast<float>(y) / static_cast<float>(surf->h);
          points.emplace_back(x - surf->w / 2, y - surf->h / 2, 0, u, v, 255, 255, 255, a);
        }
      }
    }
    printf("%lu\n", points.size());

    noiseTexture = loadTexture("loop064/rgb-perlin-seamless-512.png");
  }

  ~Test4() {
    gl.deleteTexture(noiseTexture);
  }

  void Update() {
    const float cycle_ratio = getCycleRatio();

    shader.attach();
    const float f = powf(cycle_ratio, 2);
    shader.sendFloat("cycle_ratio", 1, &f, 1);
    gl.points(points.data(), points.size(), glm::mat4());
    shader.detach();
  }

private:
  Shader shader;
  vector<Vertex> points;
  GLuint noiseTexture;
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
