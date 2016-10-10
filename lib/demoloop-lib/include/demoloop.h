#pragma once

#include <SDL.h>
#include <chrono>

class Demoloop
{
public:
  Demoloop();
  Demoloop(int r, int g, int b);
  Demoloop(int width, int height, int r, int g, int b);
  ~Demoloop();
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
