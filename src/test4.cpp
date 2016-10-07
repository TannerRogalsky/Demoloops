#include <iostream>
#include <cmath>
#include <SDL.h>
#include <SDL_ttf.h>
#include "demoloop.h"
#include "helpers.h"
#include "res_path.h"
#include "cleanup.h"
using namespace std;

float t = 0;

class Test4 : public Demoloop {
public:
  Test4() : Demoloop() {
    image = loadTexture(getResourcePath() + "image.png", renderer);

    auto font = loadFont(getResourcePath() + "04b03.TTF", 16);
    SDL_Color White = {255, 255, 255};
    text = renderText("TEST", font, White, renderer);
  }

  ~Test4() {
    cleanup(image);
  }

  void Update(float dt) {
    t += dt;

    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderClear(renderer);

    int iW, iH;
    SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
    int x = 640 / 2 - iW / 2;
    int y = 480 / 2 - iH / 2;

    x += cos(t) * 100;
    y += sin(t) * 100;

    renderTexture(image, renderer, x, y, iW, iH);
    renderTexture(text, renderer, 0, 0);


    SDL_RenderPresent(renderer);
  }

private:
  SDL_Texture *image;
  SDL_Texture *text;
};

int main(int, char**){
  Test4 test;
  test.Run();

  return 0;
}
