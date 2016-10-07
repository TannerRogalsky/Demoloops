#pragma once

class Demoloop
{
public:
  Demoloop();
  Demoloop(int r, int g, int b);
  Demoloop(int width, int height, int r, int g, int b);
  ~Demoloop();
  void Run();
  virtual void Update(float dt) = 0;
protected:
  bool quit;
  SDL_Renderer *renderer;
  SDL_Window *window;
private:
  int bg_r, bg_g, bg_b;
  void InternalUpdate(float dt);
  SDL_Event e;
};
