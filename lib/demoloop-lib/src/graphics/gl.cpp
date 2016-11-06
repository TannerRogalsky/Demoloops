#include "graphics/gl.h"
#include <string>
#include <iostream>
#include <cstddef>

const static std::string defaultVertexShader = "vec4 position(mat4 transform_proj, vec4 vertpos) {\n"
                                               "  return transform_proj * vertpos;\n"
                                               "}\n";
const static std::string defaultFragShader = "vec4 effect(mediump vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) {\n"
                                             "  return Texel(tex, texcoord) * vcolor;\n"
                                             "}\n";

namespace demoloop {
  GL::GL()
    : maxAnisotropy(1.0f)
    , maxTextureSize(0)
    , maxRenderTargets(1)
    , maxRenderbufferSamples(0)
    , maxTextureUnits(1)
    , state()
  {
    initMatrices();
  }

  GL::~GL() {}

  bool GL::initContext() {
    glEnable(GL_MULTISAMPLE); // TODO: is this doing anything?
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    initMaxValues();

    Shader::defaultShader = new Shader({defaultVertexShader, defaultFragShader});
    Shader::defaultShader->attach();

    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mIBO);

    state.boundTextures.clear();
    state.boundTextures.resize(maxTextureUnits, 0);

    for (int i = 0; i < (int) state.boundTextures.size(); i++)
    {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    glActiveTexture(GL_TEXTURE0);
    state.curTextureUnit = 0;

    createDefaultTexture();

    glBindTexture(GL_TEXTURE_2D, state.defaultTexture);
    glVertexAttrib4f(ATTRIB_CONSTANTCOLOR, 1, 1, 1, 1);
    glVertexAttrib4f(ATTRIB_COLOR, 1, 1, 1, 1);

    GLint maxvertexattribs = 1;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxvertexattribs);
    state.enabledAttribArrays = (uint32_t) ((1ull << uint32_t(maxvertexattribs)) - 1);
    useVertexAttribArrays(0);

    return true;
  }

  void GL::initMatrices() {
    matrices.transform.clear();
    matrices.projection.clear();

    matrices.transform.push_back(Matrix4());
    matrices.projection.push_back(Matrix4());
  }

  void GL::initMaxValues()
  {
    // We'll need this value to clamp anisotropy.
    if (GLEW_EXT_texture_filter_anisotropic)
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    else
      maxAnisotropy = 1.0f;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    int maxattachments = 1;
    int maxdrawbuffers = 1;

    if (GLEW_VERSION_2_0)
    {
      glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxattachments);
      glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxdrawbuffers);
    }

    maxRenderTargets = std::min(maxattachments, maxdrawbuffers);

    if (GLEW_VERSION_3_0 || GLEW_ARB_framebuffer_object
      || GLEW_EXT_framebuffer_multisample || GLEW_ANGLE_framebuffer_multisample)
    {
      glGetIntegerv(GL_MAX_SAMPLES, &maxRenderbufferSamples);
    }
    else
      maxRenderbufferSamples = 0;

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
  }

  void GL::setViewport(const GL::Viewport &v) {
    glViewport(v.x, v.y, v.w, v.h);
    state.viewport = v;
    GLfloat dimensions[4] = {(GLfloat)v.w, (GLfloat)v.h, 0, 0};
    Shader::defaultShader->sendFloat("demoloop_ScreenSize", 4, dimensions, 1);
  }

  GL::Viewport GL::getViewport() const
  {
    return state.viewport;
  }

  void GL::pushTransform()
  {
    matrices.transform.push_back(matrices.transform.back());
  }

  void GL::popTransform()
  {
    matrices.transform.pop_back();
  }

  Matrix4 &GL::getTransform()
  {
    return matrices.transform.back();
  }

  void GL::pushProjection()
  {
    matrices.projection.push_back(matrices.projection.back());
  }

  void GL::popProjection()
  {
    matrices.projection.pop_back();
  }

  Matrix4 &GL::getProjection()
  {
    return matrices.projection.back();
  }

  void GL::useVertexAttribArrays(uint32_t arraybits)
  {
    uint32_t diff = arraybits ^ state.enabledAttribArrays;

    if (diff == 0)
      return;

    // Max 32 attributes. As of when this was written, no GL driver exposes more
    // than 32. Lets hope that doesn't change...
    for (uint32_t i = 0; i < 32; i++)
    {
      uint32_t bit = 1 << i;

      if (diff & bit)
      {
        if (arraybits & bit)
          glEnableVertexAttribArray(i);
        else
          glDisableVertexAttribArray(i);
      }
    }

    state.enabledAttribArrays = arraybits;

    // glDisableVertexAttribArray will make the constant value for a vertex
    // attribute undefined. We rely on the per-vertex color attribute being
    // white when no per-vertex color is used, so we set it here.
    // FIXME: Is there a better place to do this?
    if ((diff & ATTRIBFLAG_COLOR) && !(arraybits & ATTRIBFLAG_COLOR))
      glVertexAttrib4f(ATTRIB_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
  }

  GLint GL::getGLWrapMode(Texture::WrapMode wmode)
  {
    switch (wmode)
    {
    case Texture::WRAP_CLAMP:
    default:
      return GL_CLAMP_TO_EDGE;
    case Texture::WRAP_CLAMP_ZERO:
      return GL_CLAMP_TO_BORDER;
    case Texture::WRAP_REPEAT:
      return GL_REPEAT;
    case Texture::WRAP_MIRRORED_REPEAT:
      return GL_MIRRORED_REPEAT;
    }

  }

  void GL::drawArrays(GLenum mode, GLint first, GLsizei count)
  {
    glDrawArrays(mode, first, count);
    // ++stats.drawCalls;
  }

  void GL::drawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
  {
    glDrawElements(mode, count, type, indices);
    // ++stats.drawCalls;
  }

  void GL::deleteTexture(GLuint texture)
  {
    // glDeleteTextures binds texture 0 to all texture units the deleted texture
    // was bound to before deletion.
    for (GLuint &texid : state.boundTextures)
    {
      if (texid == texture)
        texid = 0;
    }

    glDeleteTextures(1, &texture);
  }

  void GL::setTextureWrap(const Texture::Wrap &w)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getGLWrapMode(w.s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getGLWrapMode(w.t));
  }

  void GL::bindFramebuffer(GLenum target, GLuint framebuffer) {
    glBindFramebuffer(target, framebuffer);
  }

  void GL::setTextureFilter(Texture::Filter &f)
  {
    GLint gmin, gmag;

    if (f.mipmap == Texture::FILTER_NONE)
    {
      if (f.min == Texture::FILTER_NEAREST)
        gmin = GL_NEAREST;
      else // f.min == Texture::FILTER_LINEAR
        gmin = GL_LINEAR;
    }
    else
    {
      if (f.min == Texture::FILTER_NEAREST && f.mipmap == Texture::FILTER_NEAREST)
        gmin = GL_NEAREST_MIPMAP_NEAREST;
      else if (f.min == Texture::FILTER_NEAREST && f.mipmap == Texture::FILTER_LINEAR)
        gmin = GL_NEAREST_MIPMAP_LINEAR;
      else if (f.min == Texture::FILTER_LINEAR && f.mipmap == Texture::FILTER_NEAREST)
        gmin = GL_LINEAR_MIPMAP_NEAREST;
      else if (f.min == Texture::FILTER_LINEAR && f.mipmap == Texture::FILTER_LINEAR)
        gmin = GL_LINEAR_MIPMAP_LINEAR;
      else
        gmin = GL_LINEAR;
    }

    switch (f.mag)
    {
    case Texture::FILTER_NEAREST:
      gmag = GL_NEAREST;
      break;
    case Texture::FILTER_LINEAR:
    default:
      gmag = GL_LINEAR;
      break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gmin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gmag);

    // if (GLAD_EXT_texture_filter_anisotropic)
    // {
    //   f.anisotropy = std::min(std::max(f.anisotropy, 1.0f), maxAnisotropy);
    //   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, f.anisotropy);
    // }
    // else
      f.anisotropy = 1.0f;
  }

  void GL::bindTexture(GLuint texture)
  {
    if (texture != state.boundTextures[state.curTextureUnit])
    {
      state.boundTextures[state.curTextureUnit] = texture;
      glBindTexture(GL_TEXTURE_2D, texture);
    }
  }

  void GL::prepareDraw() {
    Matrix4 modelView;
    prepareDraw(modelView);
  }

  void GL::prepareDraw(Matrix4 modelView) {
    Shader::defaultShader->checkSetBuiltinUniforms();
  }

  void GL::lines(const Vertex *coords, size_t count) {
    prepareDraw();

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), &coords[0].x, GL_DYNAMIC_DRAW);

    useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR);

    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glDrawArrays(GL_LINES, 0, count);
  }

  void GL::triangles(const Vertex *coords, size_t count) {
    prepareDraw();

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), &coords[0].x, GL_DYNAMIC_DRAW);

    useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD);

    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glDrawArrays(GL_TRIANGLES, 0, count);
  }

  void GL::triangles(const Triangle *triangleVertices, size_t count) {
    triangles((Vertex *)triangleVertices, count * 3);
  }

  void GL::triangles(const Vertex *coords, const uint32_t *indices, size_t count) {
    prepareDraw();

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), &coords[0], GL_DYNAMIC_DRAW);

    useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD);

    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), &indices[0], GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
  }

  GLuint GL::getDefaultTexture() const {
    return state.defaultTexture;
  }

  void GL::createDefaultTexture()
  {
    // Set the 'default' texture (id 0) as a repeating white pixel. Otherwise,
    // texture2D calls inside a shader would return black when drawing graphics
    // primitives, which would create the need to use different "passthrough"
    // shaders for untextured primitives vs images.

    GLuint curtexture = state.boundTextures[state.curTextureUnit];

    glGenTextures(1, &state.defaultTexture);
    glBindTexture(GL_TEXTURE_2D, state.defaultTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLubyte pix[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);

    bindTexture(curtexture);
  }

  GL gl;
}
