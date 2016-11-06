#pragma once

#include <SDL.h>
#include <chrono>
#include "graphics/gl.h"
#include "graphics/canvas.h"
#include "hsl.h"

namespace demoloop {
class Demoloop {
public:
  Demoloop();
  Demoloop(int r, int g, int b);
  Demoloop(int width, int height, int r, int g, int b);
  ~Demoloop();
  void Run();
  virtual void Update(float dt) = 0;
  int width, height;
protected:
  bool quit;
  SDL_Renderer *renderer;
  SDL_Window *window;
  std::chrono::time_point<std::chrono::high_resolution_clock> previous_frame;

  void setViewportSize(int width, int height);

  void setColor(const RGB& rgb, uint8_t a = 255);
  void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

  void setCanvas(Canvas *canvas);
  // void setCanvas(const std::vector<Canvas *> &canvases);
  void setCanvas();

  std::vector<Canvas *> getCanvas() const;
private:
  int bg_r, bg_g, bg_b;
  void InternalUpdate();
  SDL_Event e;

  struct DisplayState
  {
    // Colorf color = Colorf(255.0, 255.0, 255.0, 255.0);
    // Colorf backgroundColor = Colorf(0.0, 0.0, 0.0, 255.0);

    // BlendMode blendMode = BLEND_ALPHA;
    // BlendAlpha blendAlphaMode = BLENDALPHA_MULTIPLY;

    // float lineWidth = 1.0f;
    // LineStyle lineStyle = LINE_SMOOTH;
    // LineJoin lineJoin = LINE_JOIN_MITER;

    // float pointSize = 1.0f;

    // bool scissor = false;
    // ScissorRect scissorRect = ScissorRect();

    // // Stencil.
    // CompareMode stencilCompare = COMPARE_ALWAYS;
    // int stencilTestValue = 0;

    // StrongRef<Font> font;
    // StrongRef<Shader> shader;

    std::vector<StrongRef<Canvas>> canvases;

    // ColorMask colorMask = ColorMask(true, true, true, true);

    // bool wireframe = false;

    // Texture::Filter defaultFilter = Texture::Filter();

    // Texture::FilterMode defaultMipmapFilter = Texture::FILTER_NEAREST;
    // float defaultMipmapSharpness = 0.0f;
  };

  std::vector<DisplayState> states;
};
}
