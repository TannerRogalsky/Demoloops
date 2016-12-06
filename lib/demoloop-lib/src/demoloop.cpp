#include "demoloop.h"
#include <SDL_ttf.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include "cleanup.h"
#include <glm/gtc/matrix_transform.hpp>

#ifdef EMSCRIPTEN
  #include <AL/al.h>
  #include <AL/alc.h>
#else
  #include <al.h>
  #include <alc.h>
#endif

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
    logSDLError("SDL_Init");
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
    logSDLError("CreateWindow");
    SDL_Quit();
    // return 1;
  }
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr){
    logSDLError("CreateRenderer");
    cleanup(window);
    SDL_Quit();
    // return 1;
  }

  states.reserve(10);
  states.push_back(DisplayState());

  auto contextGL = SDL_GL_CreateContext(window);
  if (contextGL == NULL) {
      logSDLError("SDL_GL_CreateContext");
  } else {
    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
      printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
    }

    gl.initContext();
    setBlendMode(BLEND_ALPHA, BLENDALPHA_MULTIPLY);
    setViewportSize(width, height);

    //Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
      logSDLError("SDL_GL_SetSwapInterval");
    }
  }

  if (TTF_Init() != 0){
    logSDLError("TTF_Init");
    SDL_Quit();
    // return 1;
  }

  ALCdevice* deviceAL = alcOpenDevice(NULL);
  ALCcontext* contextAL = alcCreateContext(deviceAL, NULL);
  alcMakeContextCurrent(contextAL);

  ALfloat listenerPos[] = {0.0, 0.0, 0.0};
  ALfloat listenerVel[] = {0.0, 0.0, 0.0};
  ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

  alListenerfv(AL_POSITION, listenerPos);
  alListenerfv(AL_VELOCITY, listenerVel);
  alListenerfv(AL_ORIENTATION, listenerOri);
}

Demoloop::~Demoloop() {
  cleanup(renderer, window);
  TTF_Quit();
  SDL_Quit();
}

void Demoloop::setColor(const RGB& rgb, uint8_t a) {
  setColor(rgb.r, rgb.g, rgb.b, a);
}

void Demoloop::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  glVertexAttrib4f(ATTRIB_CONSTANTCOLOR, r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}

void Demoloop::setCanvas(Canvas *canvas)
{
  if (canvas == nullptr)
    return setCanvas();

  DisplayState &state = states.back();

  canvas->startGrab();

  std::vector<StrongRef<Canvas>> canvasref;
  canvasref.push_back(canvas);

  std::swap(state.canvases, canvasref);
}

// void Demoloop::setCanvas(const std::vector<Canvas *> &canvases)
// {
//   if (canvases.size() == 0)
//     return setCanvas();
//   else if (canvases.size() == 1)
//     return setCanvas(canvases[0]);

//   DisplayState &state = states.back();

//   auto attachments = std::vector<Canvas *>(canvases.begin() + 1, canvases.end());
//   canvases[0]->startGrab(attachments);

//   std::vector<Canvas> canvasrefs;
//   canvasrefs.reserve(canvases.size());

//   for (Canvas *c : canvases)
//     canvasrefs.push_back(c);

//   std::swap(state.canvases, canvasrefs);
// }

void Demoloop::setCanvas()
{
  DisplayState &state = states.back();

  if (Canvas::current != nullptr)
    Canvas::current->stopGrab();

  state.canvases.clear();
}

std::vector<Canvas *> Demoloop::getCanvas() const
{
  std::vector<Canvas *> canvases;
  canvases.reserve(states.back().canvases.size());

  for (const StrongRef<Canvas> &c : states.back().canvases)
    canvases.push_back(c.get());

  return canvases;
}

void Demoloop::setViewportSize(int width, int height)
{
  this->width = width;
  this->height = height;

  // We want to affect the main screen, not any Canvas that's currently active
  // (not that any *should* be active when this is called.)
  // std::vector<StrongRef<Canvas>> canvases = states.back().canvases;
  // setCanvas();

  // Set the viewport to top-left corner.
  gl.setViewport({0, 0, width, height});

  // If a canvas was bound before this function was called, it needs to be
  // made aware of the new system viewport size.
  Canvas::systemViewport = gl.getViewport();

  // Set up the projection matrix
  gl.matrices.projection.back() = glm::ortho(0.0f, (float) width, (float) height, 0.0f, 0.1f, 100.0f);
  const glm::vec3 eye = {0, 0, 100};
  const glm::vec3 center = {0, 0, 0};
  const glm::vec3 up = {0, 1, 0};
  gl.matrices.transform.back() = glm::lookAt(eye, center, up);

  // Restore the previously active Canvas.
  // setCanvas(canvases);
}

void Demoloop::setBlendMode(BlendMode mode, BlendAlpha alphamode)
{
  GLenum func   = GL_FUNC_ADD;
  GLenum srcRGB = GL_ONE;
  GLenum srcA   = GL_ONE;
  GLenum dstRGB = GL_ZERO;
  GLenum dstA   = GL_ZERO;

  // if (mode == BLEND_LIGHTEN || mode == BLEND_DARKEN)
  // {
  //   if (!isSupported(SUPPORT_LIGHTEN))
  //     throw love::Exception("The 'lighten' and 'darken' blend modes are not supported on this system.");
  // }

  // if (alphamode != BLENDALPHA_PREMULTIPLIED)
  // {
  //   const char *modestr = "unknown";
  //   switch (mode)
  //   {
  //   case BLEND_LIGHTEN:
  //   case BLEND_DARKEN:
  //   /*case BLEND_MULTIPLY:*/ // FIXME: Uncomment for 0.11.0
  //     getConstant(mode, modestr);
  //     throw love::Exception("The '%s' blend mode must be used with premultiplied alpha.", modestr);
  //     break;
  //   default:
  //     break;
  //   }
  // }

  switch (mode)
  {
  case BLEND_ALPHA:
    srcRGB = srcA = GL_ONE;
    dstRGB = dstA = GL_ONE_MINUS_SRC_ALPHA;
    break;
  case BLEND_MULTIPLY:
    srcRGB = srcA = GL_DST_COLOR;
    dstRGB = dstA = GL_ZERO;
    break;
  case BLEND_SUBTRACT:
    func = GL_FUNC_REVERSE_SUBTRACT;
  case BLEND_ADD:
    srcRGB = GL_ONE;
    srcA = GL_ZERO;
    dstRGB = dstA = GL_ONE;
    break;
  case BLEND_LIGHTEN:
    func = GL_MAX;
    break;
  case BLEND_DARKEN:
    func = GL_MIN;
    break;
  case BLEND_SCREEN:
    srcRGB = srcA = GL_ONE;
    dstRGB = dstA = GL_ONE_MINUS_SRC_COLOR;
    break;
  case BLEND_REPLACE:
  default:
    srcRGB = srcA = GL_ONE;
    dstRGB = dstA = GL_ZERO;
    break;
  }

  // We can only do alpha-multiplication when srcRGB would have been unmodified.
  if (srcRGB == GL_ONE && alphamode == BLENDALPHA_MULTIPLY)
    srcRGB = GL_SRC_ALPHA;

  glBlendEquation(func);
  glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);

  states.back().blendMode = mode;
  states.back().blendAlphaMode = alphamode;
}

Demoloop::BlendMode Demoloop::getBlendMode(BlendAlpha &alphamode) const {
  alphamode = states.back().blendAlphaMode;
  return states.back().blendMode;
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
    const std::chrono::duration<float> interval(1.0f / 60.0f);
    while (!quit) {
      auto start = std::chrono::high_resolution_clock::now();
      InternalUpdate();
      while((std::chrono::high_resolution_clock::now() - start) < interval){}
    }
    // const std::chrono::seconds CYCLE_LENGTH(10);
    // const std::chrono::duration<float> interval(1.0f / 50.0f);
    // char* pixels = new char [3 * width * height];
    // SDL_Surface* temp = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
    // uint32_t index = 0;
    // for (std::chrono::duration<float> elapsed(0); elapsed <= CYCLE_LENGTH; elapsed += interval) {
    //   glClearColor( bg_r / 255.0, bg_g / 255.0, bg_b / 255.0, 1.f );
    //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //   Update(interval.count());

    //   SDL_GL_SwapWindow(window);

    //   glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    //   for (int i = 0 ; i < height ; i++)
    //     std::memcpy( ((char *) temp->pixels) + temp->pitch * i, pixels + 3 * width * (height-i - 1), width*3 );

    //   auto numString = std::to_string(index++);
    //   numString = std::string(4 - numString.length(), '0') + numString;
    //   SDL_SaveBMP(temp, ("frames/frame" +  numString + ".bmp").c_str());
    // }
    // cleanup(temp);
    // delete [] pixels;
  #endif
}

}
