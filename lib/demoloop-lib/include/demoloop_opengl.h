#pragma once

#include <SDL.h>
#include <chrono>

class DemoloopOpenGL
{
public:
  DemoloopOpenGL();
  DemoloopOpenGL(int r, int g, int b);
  DemoloopOpenGL(int width, int height, int r, int g, int b);
  ~DemoloopOpenGL();
  void Run();
  virtual void Update(float dt) = 0;
  const int width, height;
protected:
  bool quit;
  SDL_Renderer *renderer;
  SDL_Window *window;
  std::chrono::time_point<std::chrono::high_resolution_clock> previous_frame;
private:
  int bg_r, bg_g, bg_b;
  void InternalUpdate();
  SDL_Event e;
};
