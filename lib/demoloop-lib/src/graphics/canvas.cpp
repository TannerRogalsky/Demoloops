/**
 * Copyright (c) 2006-2016 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "graphics/canvas.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

namespace demoloop
{

static GLenum createFBO(GLuint &framebuffer, GLuint texture)
{
  // get currently bound fbo to reset to it later
  GLint current_fbo;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo);

  glGenFramebuffers(1, &framebuffer);
  gl.bindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  if (texture != 0)
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Initialize the texture to transparent black.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  // unbind framebuffer
  gl.bindFramebuffer(GL_FRAMEBUFFER, (GLuint) current_fbo);

  return status;
}

Canvas *Canvas::current = nullptr;
GL::Viewport Canvas::systemViewport = GL::Viewport();
bool Canvas::screenHasSRGB = false;
int Canvas::canvasCount = 0;

Canvas::Canvas(int width, int height, Format format, int msaa)
  : fbo(0)
  , resolve_fbo(0)
  , vbo(0)
  , texture(0)
  , msaa_buffer(0)
  , depth_stencil(0)
  , format(format)
  , requested_samples(msaa)
  , actual_samples(0)
  , texture_memory(0)
{
  this->width = width;
  this->height = height;

  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  // Vertices are ordered for use with triangle strips:
  // 0----2
  // |  / |
  // | /  |
  // 1----3
  // world coordinates
  vertices[0].x = 0;
  vertices[0].y = 0;
  vertices[0].z = 1;
  vertices[1].x = 0;
  vertices[1].y = h;
  vertices[1].z = 1;
  vertices[2].x = w;
  vertices[2].y = 0;
  vertices[2].z = 1;
  vertices[3].x = w;
  vertices[3].y = h;
  vertices[3].z = 1;

  // texture coordinates
  vertices[0].s = 0;
  vertices[0].t = 0;
  vertices[1].s = 0;
  vertices[1].t = 1;
  vertices[2].s = 1;
  vertices[2].t = 0;
  vertices[3].s = 1;
  vertices[3].t = 1;

  loadVolatile();

  ++canvasCount;
}

Canvas::~Canvas()
{
  --canvasCount;

  // reset framebuffer if still using this one
  if (current == this)
    stopGrab();

  unloadVolatile();
}

bool Canvas::loadVolatile()
{
  // GL::TempDebugGroup debuggroup("Canvas load");

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), &vertices[0].x, GL_DYNAMIC_DRAW);

  fbo = depth_stencil = texture = 0;
  resolve_fbo = msaa_buffer = 0;
  status = GL_FRAMEBUFFER_COMPLETE;

  int32_t maxTextureSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
  // glTexImage2D is guaranteed to error in this case.
  if (width > maxTextureSize || height > maxTextureSize)
  {
    status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
    return false;
  }

  // getMaxRenderbufferSamples will be 0 on systems that don't support
  // multisampled renderbuffers / don't export FBO multisample extensions.
  // requested_samples = std::min(requested_samples, gl.getMaxRenderbufferSamples());
  // requested_samples = std::max(requested_samples, 0);
  requested_samples = 0; // ES2 == no MSAA frame buffers :(

  glGenTextures(1, &texture);
  gl.bindTexture(texture);

  // if (GLAD_ANGLE_texture_usage)
  //   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_USAGE_ANGLE, GL_FRAMEBUFFER_ATTACHMENT_ANGLE);

  setFilter(filter);
  setWrap(wrap);

  GLenum internalformat = GL_RGBA;
  GLenum externalformat = GL_RGBA;
  GLenum textype = GL_UNSIGNED_BYTE;

  convertFormat(format, internalformat, externalformat, textype);

  // in GLES2, the internalformat and format params of TexImage have to match.
  // GLint iformat = (GLint) internalformat;
  // if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
  GLint iformat = (GLint) externalformat;

  while (glGetError() != GL_NO_ERROR)
    /* Clear the error buffer. */;

  glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0,
               externalformat, textype, nullptr);

  if (glGetError() != GL_NO_ERROR)
  {
    gl.deleteTexture(texture);
    texture = 0;
    status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
    return false;
  }

  // Try to create a MSAA FBO if requested. On failure (or no requested MSAA),
  // fall back to a regular FBO.
  status = createFBO(fbo, texture);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    if (fbo != 0) {
      glDeleteFramebuffers(1, &fbo);
      fbo = 0;
    }
    return false;
  }

  // size_t prevmemsize = texture_memory;

  // texture_memory = (getFormatBitsPerPixel(format) * width * height) / 8;
  // if (msaa_buffer != 0)
  //   texture_memory += (texture_memory * actual_samples);

  // gl.updateTextureMemorySize(prevmemsize, texture_memory);

  return true;
}

void Canvas::unloadVolatile()
{
  glDeleteFramebuffers(1, &fbo);
  glDeleteFramebuffers(1, &resolve_fbo);

  glDeleteRenderbuffers(1, &depth_stencil);
  glDeleteRenderbuffers(1, &msaa_buffer);

  gl.deleteTexture(texture);

  fbo = 0;
  resolve_fbo = 0;
  depth_stencil = 0;
  msaa_buffer = 0;
  texture = 0;

  attachedCanvases.clear();

  // gl.updateTextureMemorySize(texture_memory, 0);
  texture_memory = 0;
}

void Canvas::drawv(const glm::mat4 &modelTransform, const Vertex *v)
{
  // FIXME: This doesn't handle cases where the Canvas is used as a texture
  // in a SpriteBatch, Mesh, or ParticleSystem, or when the Canvas is used in
  // a shader as a non-default texture.
  if (Canvas::current == this) {
    throw std::runtime_error("Cannot draw a Canvas to itself.");
  }

  // GL::TempDebugGroup debuggroup("Canvas draw");

  gl.bindTexture(texture);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), &v[0].x, GL_DYNAMIC_DRAW);

  gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD);

  glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
  glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));

  gl.prepareDraw(modelTransform);
  gl.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Canvas::draw(glm::mat4 curModel)
{
  drawv(curModel, vertices);
}

void Canvas::drawq(Quad *quad, glm::mat4 curModel)
{
  drawv(curModel, quad->getVertices());
}

void Canvas::setFilter(const Texture::Filter &f)
{
  if (!validateFilter(f, false))
    throw std::runtime_error("Invalid texture filter.");

  filter = f;
  gl.bindTexture(texture);
  gl.setTextureFilter(filter);
}

bool Canvas::setWrap(const Texture::Wrap &w)
{
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

const void *Canvas::getHandle() const
{
  return &texture;
}

void Canvas::setupGrab()
{
  // already grabbing
  if (current == this)
    return;

  // cleanup after previous Canvas
  if (current != nullptr)
  {
    systemViewport = current->systemViewport;
    current->stopGrab(true);
  }
  else
    systemViewport = gl.getViewport();

  // indicate we are using this Canvas.
  current = this;

  // bind the framebuffer object.
  gl.bindFramebuffer(GL_FRAMEBUFFER, fbo);
  gl.setViewport({0, 0, width, height});

  // Set up the projection matrix
  gl.matrices.projection.push_back(glm::ortho(0.0f, (float) width, 0.0f, (float) height, 0.1f, 100.0f));
}

void Canvas::startGrab()
{
  setupGrab();

  if (attachedCanvases.size() == 0)
    return;

  // // Make sure the FBO is only using a single draw buffer.
  // glDrawBuffer(GL_COLOR_ATTACHMENT0);

  attachedCanvases.clear();
}

void Canvas::stopGrab(bool switchingToOtherCanvas)
{
  // i am not grabbing. leave me alone
  if (current != this)
    return;

  // GL::TempDebugGroup debuggroup("Canvas un-set");

  // Make sure the canvas texture is up to date if we're using MSAA.
  // resolveMSAA(false);

  gl.matrices.projection.pop_back();

  if (!switchingToOtherCanvas)
  {
    // bind system framebuffer.
    // gl.bindFramebuffer(GL_FRAMEBUFFER, gl.getDefaultFBO());
    gl.bindFramebuffer(GL_FRAMEBUFFER, 0);
    current = nullptr;
    gl.setViewport(systemViewport);

    // if (GLAD_VERSION_1_0 || GLAD_EXT_sRGB_write_control)
    // {
    //   if (screenHasSRGB && !gl.hasFramebufferSRGB())
    //     gl.setFramebufferSRGB(true);
    //   else if (!screenHasSRGB && gl.hasFramebufferSRGB())
    //     gl.setFramebufferSRGB(false);
    // }
  }
}

Canvas::Format Canvas::getSizedFormat(Canvas::Format format)
{
  switch (format)
  {
  case FORMAT_NORMAL:
    return FORMAT_RGBA8;
  case FORMAT_HDR:
    return FORMAT_RGBA16F;
  default:
    return format;
  }
}

void Canvas::convertFormat(Canvas::Format format, GLenum &internalformat, GLenum &externalformat, GLenum &type)
{
  format = getSizedFormat(format);
  externalformat = GL_RGBA;

  switch (format)
  {
  case FORMAT_RGBA4:
    internalformat = GL_RGBA4;
    type = GL_UNSIGNED_SHORT_4_4_4_4;
    break;
  case FORMAT_RGB5A1:
    internalformat = GL_RGB5_A1;
    type = GL_UNSIGNED_SHORT_5_5_5_1;
    break;
  case FORMAT_R8:
    internalformat = GL_R8;
    externalformat = GL_RED;
    type = GL_UNSIGNED_BYTE;
    break;
  case FORMAT_RG8:
    internalformat = GL_RG8;
    externalformat = GL_RG;
    type = GL_UNSIGNED_BYTE;
    break;
  case FORMAT_RGBA8:
  default:
    internalformat = GL_RGBA8;
    type = GL_UNSIGNED_BYTE;
    break;
  case FORMAT_RGB10A2:
    internalformat = GL_RGB10_A2;
    type = GL_UNSIGNED_INT_2_10_10_10_REV;
    break;
  case FORMAT_RG11B10F:
    internalformat = GL_R11F_G11F_B10F;
    externalformat = GL_RGB;
    type = GL_UNSIGNED_INT_10F_11F_11F_REV;
    break;
  case FORMAT_R32F:
    internalformat = GL_R32F;
    externalformat = GL_RED;
    type = GL_FLOAT;
    break;
  case FORMAT_RG32F:
    internalformat = GL_RG32F;
    externalformat = GL_RG;
    type = GL_FLOAT;
    break;
  case FORMAT_RGBA32F:
    internalformat = GL_RGBA32F;
    type = GL_FLOAT;
    break;
  case FORMAT_SRGB:
    internalformat = GL_SRGB8_ALPHA8;
    type = GL_UNSIGNED_BYTE;
    #ifdef EMSCRIPTEN
      externalformat = GL_SRGB_ALPHA;
    #endif
    break;
  }
}

size_t Canvas::getFormatBitsPerPixel(Format format)
{
  switch (getSizedFormat(format))
  {
  case FORMAT_R8:
    return 8;
  case FORMAT_RGBA4:
  case FORMAT_RGB5A1:
  case FORMAT_RGB565:
  case FORMAT_RG8:
  case FORMAT_R16F:
    return 16;
  case FORMAT_RGBA8:
  case FORMAT_RGB10A2:
  case FORMAT_RG11B10F:
  case FORMAT_RG16F:
  case FORMAT_R32F:
  case FORMAT_SRGB:
  default:
    return 32;
  case FORMAT_RGBA16F:
  case FORMAT_RG32F:
    return 64;
  case FORMAT_RGBA32F:
    return 128;
  }
}

bool Canvas::isSupported()
{
  // return GLAD_ES_VERSION_2_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object || GLAD_EXT_framebuffer_object;
  return true;
}

bool Canvas::isMultiFormatMultiCanvasSupported()
{
  // return gl.getMaxRenderTargets() > 1 && (GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object);
  return false;
}

bool Canvas::supportedFormats[] = {false};
bool Canvas::checkedFormats[] = {false};

bool Canvas::isFormatSupported(Canvas::Format format)
{
  if (!isSupported())
    return false;

  bool supported = true;
  format = getSizedFormat(format);

  switch (format)
  {
  case FORMAT_RGBA4:
  case FORMAT_RGB5A1:
    supported = true;
    break;
  // case FORMAT_RGB565:
  //   supported = GLAD_ES_VERSION_2_0 || GLAD_VERSION_4_2 || GLAD_ARB_ES2_compatibility;
  //   break;
  // case FORMAT_R8:
  // case FORMAT_RG8:
  //   if (GLAD_VERSION_1_0)
  //     supported = GLAD_VERSION_3_0 || GLAD_ARB_texture_rg;
  //   else if (GLAD_ES_VERSION_2_0)
  //     supported = GLAD_ES_VERSION_3_0 || GLAD_EXT_texture_rg;
  //   break;
  case FORMAT_RGBA8:
    // supported = GLAD_VERSION_1_0 || GLAD_ES_VERSION_3_0 || GLAD_OES_rgb8_rgba8 || GLAD_ARM_rgba8;
    supported = true;
    break;
  // case FORMAT_RGB10A2:
  //   supported = GLAD_ES_VERSION_3_0 || GLAD_VERSION_1_0;
  //   break;
  // case FORMAT_RG11B10F:
  //   supported = GLAD_VERSION_3_0 || GLAD_EXT_packed_float || GLAD_APPLE_color_buffer_packed_float;
  //   break;
  // case FORMAT_R16F:
  // case FORMAT_RG16F:
  //   if (GLAD_VERSION_1_0)
  //     supported = GLAD_VERSION_3_0 || (GLAD_ARB_texture_float && GLAD_ARB_texture_rg);
  //   else
  //     supported = GLAD_EXT_color_buffer_half_float && (GLAD_ES_VERSION_3_0 || (GLAD_OES_texture_half_float && GLAD_EXT_texture_rg));
  //   break;
  // case FORMAT_RGBA16F:
  //   if (GLAD_VERSION_1_0)
  //     supported = GLAD_VERSION_3_0 || GLAD_ARB_texture_float;
  //   else if (GLAD_ES_VERSION_2_0)
  //     supported = GLAD_EXT_color_buffer_half_float && (GLAD_ES_VERSION_3_0 || GLAD_OES_texture_half_float);
  //   break;
  // case FORMAT_R32F:
  // case FORMAT_RG32F:
  //   supported = GLAD_VERSION_3_0 || (GLAD_ARB_texture_float && GLAD_ARB_texture_rg);
  //   break;
  // case FORMAT_RGBA32F:
  //   supported = GLAD_VERSION_3_0 || GLAD_ARB_texture_float;
  //   break;
  // case FORMAT_SRGB:
  //   if (GLAD_VERSION_1_0)
  //   {
  //     supported = GLAD_VERSION_3_0 || ((GLAD_ARB_framebuffer_sRGB || GLAD_EXT_framebuffer_sRGB)
  //       && (GLAD_VERSION_2_1 || GLAD_EXT_texture_sRGB));
  //   }
  //   else
  //     supported = GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB;
  //   break;
  default:
    supported = false;
    break;
  }

  if (!supported)
    return false;

  if (checkedFormats[format])
    return supportedFormats[format];

  // Even though we might have the necessary OpenGL version or extension,
  // drivers are still allowed to throw FRAMEBUFFER_UNSUPPORTED when attaching
  // a texture to a FBO whose format the driver doesn't like. So we should
  // test with an actual FBO.

  GLenum internalformat = GL_RGBA;
  GLenum externalformat = GL_RGBA;
  GLenum textype = GL_UNSIGNED_BYTE;
  convertFormat(format, internalformat, externalformat, textype);

  // in GLES2, the internalformat and format params of TexImage have to match.
  // if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
  #ifdef EMSCRIPTEN
    internalformat = externalformat;
  #endif

  GLuint texture = 0;
  glGenTextures(1, &texture);
  gl.bindTexture(texture);

  Texture::Filter f;
  f.min = f.mag = Texture::FILTER_NEAREST;
  gl.setTextureFilter(f);

  Texture::Wrap w;
  gl.setTextureWrap(w);

  glTexImage2D(GL_TEXTURE_2D, 0, internalformat, 2, 2, 0, externalformat, textype, nullptr);

  GLuint fbo = 0;
  supported = (createFBO(fbo, texture) == GL_FRAMEBUFFER_COMPLETE);
  glDeleteFramebuffers(1, &fbo);

  gl.deleteTexture(texture);

  // Cache the result so we don't do this for every isFormatSupported call.
  checkedFormats[format] = true;
  supportedFormats[format] = supported;

  return supported;
}

bool Canvas::getConstant(const char *in, Format &out)
{
  return formats.find(in, out);
}

bool Canvas::getConstant(Format in, const char *&out)
{
  return formats.find(in, out);
}

StringMap<Canvas::Format, Canvas::FORMAT_MAX_ENUM>::Entry Canvas::formatEntries[] =
{
  {"normal", FORMAT_NORMAL},
  {"hdr", FORMAT_HDR},
  {"rgba4", FORMAT_RGBA4},
  {"rgb5a1", FORMAT_RGB5A1},
  {"rgb565", FORMAT_RGB565},
  {"r8", FORMAT_R8},
  {"rg8", FORMAT_RG8},
  {"rgba8", FORMAT_RGBA8},
  {"rgb10a2", FORMAT_RGB10A2},
  {"rg11b10f", FORMAT_RG11B10F},
  {"r16f", FORMAT_R16F},
  {"rg16f", FORMAT_RG16F},
  {"rgba16f", FORMAT_RGBA16F},
  {"r32f", FORMAT_R32F},
  {"rg32f", FORMAT_RG32F},
  {"rgba32f", FORMAT_RGBA32F},
  {"srgb", FORMAT_SRGB},
};

StringMap<Canvas::Format, Canvas::FORMAT_MAX_ENUM> Canvas::formats(Canvas::formatEntries, sizeof(Canvas::formatEntries));

} // love
