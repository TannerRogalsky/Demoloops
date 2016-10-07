#pragma once

class Demoloop
{
public:
  Demoloop();
  ~Demoloop();
  void Run();
  virtual void Update(float dt) = 0;
protected:
  bool quit;
  SDL_Renderer *renderer;
  SDL_Window *window;
private:
  void InternalUpdate(float dt);
  SDL_Event e;
};
