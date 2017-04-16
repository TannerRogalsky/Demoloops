#include "graphics/image.h"
#include "res_path.h"
#include <SDL_image.h>
#include <stdexcept>

namespace demoloop {

Image::Image(const std::string &path) {
  SDL_Surface *surf = IMG_Load((getResourcePath() + path).c_str());
  if (surf == nullptr) {
    logSDLError("IMG_Load");
  }

  auto fmt = surf->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
  width = surf->w;
  height = surf->h;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D,texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w,surf->h, 0, fmt, GL_UNSIGNED_BYTE,surf->pixels);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  SDL_FreeSurface(surf);

  for (int i = 0; i < 4; i++)
    vertices[i].r = vertices[i].g = vertices[i].b = vertices[i].a = 255;

  // Vertices are ordered for use with triangle strips:
  // 0----2
  // |  / |
  // | /  |
  // 1----3
  vertices[0].x = 0.0f;
  vertices[0].y = 0.0f;
  vertices[1].x = 0.0f;
  vertices[1].y = (float) height;
  vertices[2].x = (float) width;
  vertices[2].y = 0.0f;
  vertices[3].x = (float) width;
  vertices[3].y = (float) height;

  vertices[0].s = 0.0f;
  vertices[0].t = 0.0f;
  vertices[1].s = 0.0f;
  vertices[1].t = 1.0f;
  vertices[2].s = 1.0f;
  vertices[2].t = 0.0f;
  vertices[3].s = 1.0f;
  vertices[3].t = 1.0f;
}

Image::~Image() {
  gl.deleteTexture(texture);
}

const void *Image::getHandle() const {
  return &texture;
}

void Image::setFilter(const Texture::Filter &f) {
  if (!validateFilter(f, false))
    throw std::runtime_error("Invalid texture filter.");

  filter = f;
  gl.bindTexture(texture);
  gl.setTextureFilter(filter);
}

bool Image::setWrap(const Texture::Wrap &w) {
  bool success = true;
  wrap = w;

  // if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
  //   && (width != next_p2(width) || height != next_p2(height)))
  if (width != next_p2(width) || height != next_p2(height))
  {
    if (wrap.s != WRAP_CLAMP || wrap.t != WRAP_CLAMP)
      success = false;

    // If we only have limited NPOT support then the wrap mode must be CLAMP.
    wrap.s = wrap.t = WRAP_CLAMP;
  }

  // if (!gl.isClampZeroTextureWrapSupported())
  // {
    if (wrap.s == WRAP_CLAMP_ZERO)
      wrap.s = WRAP_CLAMP;
    if (wrap.t == WRAP_CLAMP_ZERO)
      wrap.t = WRAP_CLAMP;
  // }

  gl.bindTexture(texture);
  gl.setTextureWrap(wrap);

  return success;
}

void Image::drawv(const glm::mat4 &modelTransform, const Vertex *v) {
  gl.bindTexture(texture);

  gl.bufferVertices(v, 4, GL_DYNAMIC_DRAW);

  gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD);
  glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
  glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));

  gl.prepareDraw(modelTransform);
  gl.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Image::draw(glm::mat4 curModel) {
  drawv(curModel, vertices);
}

void Image::drawq(Quad *quad, glm::mat4 curModel) {
  drawv(curModel, quad->getVertices());
}

}
