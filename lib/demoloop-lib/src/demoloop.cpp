#include <iostream>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include "demoloop.h"
#include "helpers.h"
#include "opengl_helpers.h"
#include "cleanup.h"

#ifdef EMSCRIPTEN
  #include <emscripten.h>
#endif

namespace demoloop {

const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;

Demoloop::Demoloop() : Demoloop(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0) {}
Demoloop::Demoloop(int r, int g, int b) : Demoloop(SCREEN_WIDTH, SCREEN_HEIGHT, r, g, b) {}

// implementation of constructor
Demoloop::Demoloop(int width, int height, int r, int g, int b)
 :width(width), height(height), quit(false), bg_r(r), bg_g(g), bg_b(b) {

  if (SDL_Init(SDL_INIT_VIDEO) != 0){
    logSDLError(std::cerr, "SDL_Init");
    // return 1;
  }

  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

  const auto WINDOW_FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
  window = SDL_CreateWindow("Demoloop", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WINDOW_FLAGS);
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

  auto context = SDL_GL_CreateContext(window);
  if (context == NULL) {
      logSDLError(std::cerr, "SDL_GL_CreateContext");
  } else {
    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
      std::cerr << "Error initializing GLEW! " << glewGetErrorString(glewError) << std::endl;
    }

    gl.initContext();
    gl.setViewport({0, 0, width, height});

    //Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
      logSDLError(std::cerr, "SDL_GL_SetSwapInterval");
    }
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

void Demoloop::setColor(const RGB& rgb, uint8_t a) {
  setColor(rgb.r, rgb.g, rgb.b, a);
}

void Demoloop::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  glVertexAttrib4f(ATTRIB_CONSTANTCOLOR, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
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

  glClearColor( bg_r / 255.0, bg_g / 255.0, bg_b / 255.0, 1.f );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  auto now = std::chrono::high_resolution_clock::now();
  auto delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - previous_frame);
  Update(delta.count());
  previous_frame = std::chrono::high_resolution_clock::now();

  SDL_GL_SwapWindow(window);
}

void Demoloop::Run() {
  previous_frame = std::chrono::high_resolution_clock::now();
  #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg([](void *arg) {
      Demoloop *self = static_cast<Demoloop*>(arg);
      self->InternalUpdate();
    }, (void *)this, 0, 1);
  #else
    while (!quit) {
      // auto start = std::chrono::high_resolution_clock::now();
      InternalUpdate();
      // while((std::chrono::high_resolution_clock::now() - start).count() < 1.0f / FRAMES_PER_SECOND ){}
    }
  #endif
}

}
