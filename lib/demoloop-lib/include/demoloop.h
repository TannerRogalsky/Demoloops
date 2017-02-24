#pragma once

#include <glm/fwd.hpp>
#include <SDL.h>
#include <chrono>
#include "graphics/gl.h"
#include "graphics/canvas.h"
#include "helpers.h"

namespace demoloop {
class Demoloop {
public:

  enum BlendMode
  {
    BLEND_ALPHA,
    BLEND_ADD,
    BLEND_SUBTRACT,
    BLEND_MULTIPLY,
    BLEND_LIGHTEN,
    BLEND_DARKEN,
    BLEND_SCREEN,
    BLEND_REPLACE,
    BLEND_MAX_ENUM
  };

  enum BlendAlpha
  {
    BLENDALPHA_MULTIPLY,
    BLENDALPHA_PREMULTIPLIED,
    BLENDALPHA_MAX_ENUM
  };

  Demoloop();
  Demoloop(int r, int g, int b);
  Demoloop(int width, int height, int r, int g, int b);
  ~Demoloop();
  void Run();
  virtual void Update(float dt) = 0;
  int getMouseX() const;
  int getMouseY() const;
  int getMouseDeltaX() const;
  int getMouseDeltaY() const;
  bool isMouseDown(uint8_t button) const;
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

  /**
   * Sets the current blend mode.
   **/
  void setBlendMode(BlendMode mode, BlendAlpha alphamode);

  /**
   * Gets the current blend mode.
   **/
  BlendMode getBlendMode(BlendAlpha &alphamode) const;

  std::vector<Canvas *> getCanvas() const;
private:
  int bg_r, bg_g, bg_b;
  void InternalUpdate();
  SDL_Event e;
  int mouse_x, mouse_y;
  int prev_mouse_x, prev_mouse_y;
  uint32_t mouse_state;

  struct DisplayState
  {
    // Colorf color = Colorf(255.0, 255.0, 255.0, 255.0);
    // Colorf backgroundColor = Colorf(0.0, 0.0, 0.0, 255.0);

    BlendMode blendMode = BLEND_ALPHA;
    BlendAlpha blendAlphaMode = BLENDALPHA_MULTIPLY;

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
