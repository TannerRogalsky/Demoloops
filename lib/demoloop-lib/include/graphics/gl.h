#pragma once

#include <GL/glew.h>
#include <vector>
#include "common/matrix.h"
#include "graphics/shader.h"
#include "graphics/texture.h"

namespace demoloop {
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
    std::vector<Matrix4> transform;
    std::vector<Matrix4> projection;
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

    Matrix4 &get()
    {
      return gl.getTransform();
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
  Matrix4 &getTransform();

  void pushProjection();
  void popProjection();
  Matrix4 &getProjection();

  /**
   * Sets the enabled vertex attribute arrays based on the specified attribute
   * bits. Each bit in the uint32_t represents an enabled attribute array index.
   * For example, useVertexAttribArrays(1 << 0) will enable attribute index 0.
   * See the VertexAttribFlags enum for the standard vertex attributes.
   * This function *must* be used instead of glEnable/DisableVertexAttribArray.
   **/
  void useVertexAttribArrays(uint32_t arraybits);

  void prepareDraw();
  void prepareDraw(Matrix4 modelView);

  /**
   * glDrawArrays and glDrawElements which increment the draw-call counter by
   * themselves.
   **/
  void drawArrays(GLenum mode, GLint first, GLsizei count);
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

  void triangles(const Vertex *coords, size_t count);
  void triangles(const Triangle* triangles, size_t count);
  void triangles(const Vertex* triangles, const uint32_t *indices, size_t count);
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

    Matrix4 lastProjectionMatrix;
    Matrix4 lastTransformMatrix;

  } state;

};

extern GL gl;
}
