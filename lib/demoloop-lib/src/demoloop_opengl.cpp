#include <iostream>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include "demoloop_opengl.h"
#include "helpers.h"
#include "opengl_helpers.h"
#include "cleanup.h"

#ifdef EMSCRIPTEN
  #include <emscripten.h>
#endif

const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;
const int FRAMES_PER_SECOND = 60;

DemoloopOpenGL::DemoloopOpenGL() : DemoloopOpenGL(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0) {}
DemoloopOpenGL::DemoloopOpenGL(int r, int g, int b) : DemoloopOpenGL(SCREEN_WIDTH, SCREEN_HEIGHT, r, g, b) {}

// implementation of constructor
DemoloopOpenGL::DemoloopOpenGL(int width, int height, int r, int g, int b)
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
  window = SDL_CreateWindow("DemoloopOpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, WINDOW_FLAGS);
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

    glEnable(GL_MULTISAMPLE); // TODO: is this doing anything?

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

DemoloopOpenGL::~DemoloopOpenGL() {
  cleanup(renderer, window);
  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
}

void DemoloopOpenGL::InternalUpdate() {
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

  // SDL_SetRenderDrawColor(renderer, bg_r, bg_g, bg_b, 255);
  // SDL_RenderClear(renderer);
  glClearColor( bg_r / 255.0, bg_g / 255.0, bg_b / 255.0, 1.f );
  glClear(GL_COLOR_BUFFER_BIT);

  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::microseconds delta = std::chrono::duration_cast<std::chrono::microseconds>(now - previous_frame);
  Update(delta.count() / 1000000.0);
  previous_frame = std::chrono::high_resolution_clock::now();

  // SDL_RenderPresent(renderer);
  SDL_GL_SwapWindow(window);
}

void DemoloopOpenGL::Run() {
  previous_frame = std::chrono::high_resolution_clock::now();
  #ifdef __EMSCRIPTEN__
    // void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
    // emscripten_set_main_loop(InternalUpdate, -1, 1);
    emscripten_set_main_loop_arg([](void *arg) {
      DemoloopOpenGL *self = (DemoloopOpenGL*)arg;
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
