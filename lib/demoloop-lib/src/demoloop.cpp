#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "demoloop.h"
#include "helpers.h"
#include "cleanup.h"

#ifdef EMSCRIPTEN
  #include <emscripten.h>
#endif

const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;
const int FRAMES_PER_SECOND = 60;

Demoloop::Demoloop() : Demoloop(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0) {}
Demoloop::Demoloop(int r, int g, int b) : Demoloop(SCREEN_WIDTH, SCREEN_HEIGHT, r, g, b) {}

// implementation of constructor
Demoloop::Demoloop(int width, int height, int r, int g, int b)
 :quit(false), width(width), height(height), bg_r(r), bg_g(g), bg_b(b) {

  if (SDL_Init(SDL_INIT_VIDEO) != 0){
    logSDLError(std::cerr, "SDL_Init");
    // return 1;
  }

  window = SDL_CreateWindow("Demoloop", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
  if (window == nullptr){
    logSDLError(std::cerr, "CreateWindow");
    SDL_Quit();
    // return 1;
  }
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr){
    logSDLError(std::cerr, "CreateRenderer");
    cleanup(window);
    SDL_Quit();
    // return 1;
  }

  if (TTF_Init() != 0){
    logSDLError(std::cerr, "TTF_Init");
    SDL_Quit();
    // return 1;
  }
}

Demoloop::~Demoloop() {
  cleanup(renderer, window);
  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
}

void Demoloop::InternalUpdate() {
  while (SDL_PollEvent(&e)){
    //If user closes the window
    if (e.type == SDL_QUIT){
      quit = true;
    }
    //If user presses any key
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE){
      quit = true;
    }
  }

  SDL_SetRenderDrawColor(renderer, bg_r, bg_g, bg_b, 255);
  SDL_RenderClear(renderer);

  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::microseconds delta = std::chrono::duration_cast<std::chrono::microseconds>(now - previous_frame);
  Update(delta.count() / 1000000.0);
  previous_frame = std::chrono::high_resolution_clock::now();

  SDL_RenderPresent(renderer);
}

void Demoloop::Run() {
  previous_frame = std::chrono::high_resolution_clock::now();
  #ifdef __EMSCRIPTEN__
    // void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
    // emscripten_set_main_loop(InternalUpdate, -1, 1);
    emscripten_set_main_loop_arg([](void *arg) {
      Demoloop *self = (Demoloop*)arg;
      self->InternalUpdate();
    }, (void *)this, 0, 1);
  #else
    while (!quit) {
      InternalUpdate();
      // Delay to keep frame rate constant (using SDL)
      SDL_Delay(1.0/FRAMES_PER_SECOND);
    }
  #endif
}
