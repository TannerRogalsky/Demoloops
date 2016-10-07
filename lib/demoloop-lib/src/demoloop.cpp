#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "demoloop.h"
#include "helpers.h"
#include "cleanup.h"

const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;

// implementation of constructor
Demoloop::Demoloop()
 :quit(false) {

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
    logSDLError(std::cerr, "SDL_Init");
    // return 1;
  }

  window = SDL_CreateWindow("Lesson 2", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
  SDL_Quit();
}

void Demoloop::InternalUpdate(float dt) {
  while (SDL_PollEvent(&e)){
    //If user closes the window
    if (e.type == SDL_QUIT){
      quit = true;
    }
    //If user presses any key
    if (e.type == SDL_KEYDOWN){
      quit = true;
    }
    //If user clicks the mouse
    if (e.type == SDL_MOUSEBUTTONDOWN){
      quit = true;
    }
  }

  Update(dt);
}

void Demoloop::Run() {
  #ifdef __EMSCRIPTEN__
    // void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
    emscripten_set_main_loop(InternalUpdate, -1, 1);
  #else
    while (!quit) {
      InternalUpdate(1.0/60.0);
      // Delay to keep frame rate constant (using SDL)
      SDL_Delay(16);
    }
  #endif
}
