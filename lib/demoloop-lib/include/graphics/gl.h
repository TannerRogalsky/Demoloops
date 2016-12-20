#pragma once

#include <GL/glew.h>
#include <vector>
#include "graphics/texture.h"

namespace demoloop {

// Vertex attribute indices used in shaders. The values map to OpenGL generic vertex attribute indices.
enum VertexAttribID
{
  ATTRIB_POS = 0,
  ATTRIB_TEXCOORD,
  ATTRIB_COLOR,
  ATTRIB_CONSTANTCOLOR,
  ATTRIB_MAX_ENUM
};

enum VertexAttribFlags
{
  ATTRIBFLAG_POS = 1 << ATTRIB_POS,
  ATTRIBFLAG_TEXCOORD = 1 << ATTRIB_TEXCOORD,
  ATTRIBFLAG_COLOR = 1 << ATTRIB_COLOR,
  ATTRIBFLAG_CONSTANTCOLOR = 1 << ATTRIB_CONSTANTCOLOR
};

class GL {
public:

  // A rectangle representing an OpenGL viewport or a scissor box.
  struct Viewport
  {
    int x, y;
    int w, h;

    bool operator == (const Viewport &rhs) const
    {
      return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
    }
  };

  struct
  {
    std::vector<glm::mat4> transform;
    std::vector<glm::mat4> projection;
  } matrices;

  class TempTransform
  {
  public:

    TempTransform(GL &gl)
      : gl(gl)
    {
      gl.pushTransform();
    }

    ~TempTransform()
    {
      gl.popTransform();
    }

    glm::mat4 &get()
    {
      return gl.getTransform();
    }

  private:
    GL &gl;
  };

  class TempProjection
  {
  public:

    TempProjection(GL &gl)
      : gl(gl)
    {
      gl.pushProjection();
    }

    ~TempProjection()
    {
      gl.popProjection();
    }

    glm::mat4 &get()
    {
      return gl.getProjection();
    }

  private:
    GL &gl;
  };

  GL();
  ~GL();

  bool initContext();

  /**
   * Sets the OpenGL rendering viewport to the specified rectangle.
   * The y-coordinate starts at the top.
   **/
  void setViewport(const Viewport &v);

  /**
   * Gets the current OpenGL rendering viewport rectangle.
   **/
  Viewport getViewport() const;

  void pushTransform();
  void popTransform();
  glm::mat4 &getTransform();

  void pushProjection();
  void popProjection();
  glm::mat4 &getProjection();

  /**
   * Sets the enabled vertex attribute arrays based on the specified attribute
   * bits. Each bit in the uint32_t represents an enabled attribute array index.
   * For example, useVertexAttribArrays(1 << 0) will enable attribute index 0.
   * See the VertexAttribFlags enum for the standard vertex attributes.
   * This function *must* be used instead of glEnable/DisableVertexAttribArray.
   **/
  void useVertexAttribArrays(uint32_t arraybits);

  void prepareDraw();
  void prepareDraw(const glm::mat4 &modelView);

  /**
   * glDrawArrays and glDrawElements which increment the draw-call counter by
   * themselves.
   **/
  void drawArrays(GLenum mode, GLint first, GLsizei count);
  void drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
  void drawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);

  /**
   * Binds a Framebuffer Object to the specified target.
   **/
  void bindFramebuffer(GLenum target, GLuint framebuffer);

  /**
   * Helper for binding an OpenGL texture.
   * Makes sure we aren't redundantly binding textures.
   **/
  void bindTexture(GLuint texture);

  /**
   * Helper for deleting an OpenGL texture.
   * Cleans up if the texture is currently bound.
   **/
  void deleteTexture(GLuint texture);

  /**
   * Sets the texture filter mode for the currently bound texture.
   * The anisotropy parameter of the argument is set to the actual amount of
   * anisotropy that was used.
   **/
  void setTextureFilter(Texture::Filter &f);

  /**
   * Sets the texture wrap mode for the currently bound texture.
   **/
  void setTextureWrap(const Texture::Wrap &w);

  GLuint getDefaultTexture() const;

  void bufferVertices(const Vertex *vertices, size_t count, GLenum usage = GL_DYNAMIC_DRAW);
  void bufferIndices(const uint32_t *indices, size_t count, GLenum usage = GL_DYNAMIC_DRAW);

  void triangles(const Vertex *coords, size_t count, const glm::mat4 &modelView);
  void triangles(const Vertex *coords, size_t count);
  void triangles(const Triangle* triangles, size_t count);
  void lines(const Vertex *coords, size_t count);

private:

  void initMatrices();
  void initMaxValues();
  void createDefaultTexture();

  GLfloat maxAnisotropy;
  GLint maxTextureSize;
  GLint maxRenderTargets;
  GLint maxRenderbufferSamples;
  GLint maxTextureUnits;

  GLint getGLWrapMode(Texture::WrapMode wmode);

  GLuint mVBO;
  GLuint mIBO;

  // Tracked OpenGL state.
  struct
  {
    // Texture unit state (currently bound texture for each texture unit.)
    std::vector<GLuint> boundTextures;

    // Currently active texture unit.
    int curTextureUnit;

    uint32_t enabledAttribArrays;

    Viewport viewport;
    Viewport scissor;

    float pointSize;

    bool framebufferSRGBEnabled;

    GLuint defaultTexture;

    glm::mat4 lastProjectionMatrix;
    glm::mat4 lastTransformMatrix;

  } state;

};

extern GL gl;
}
