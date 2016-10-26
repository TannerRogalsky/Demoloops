#pragma once

#include <GL/glew.h>
#include <vector>
#include "common/matrix.h"
#include "graphics/shader.h"

namespace Demoloop {
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

  GL();
  ~GL();

  bool initContext();

  /**
   * Sets the OpenGL rendering viewport to the specified rectangle.
   * The y-coordinate starts at the top.
   **/
  void setViewport(const Viewport &v);

  void pushTransform();
  void popTransform();
  Matrix4 &getTransform();

  void pushProjection();
  void popProjection();
  Matrix4 &getProjection();

  void prepareDraw();

  void triangles(const Vertex *coords, size_t count);
  void triangles(const Triangle* triangles, size_t count);
  void triangles(const Vertex* triangles, const uint32_t *indices, size_t count);
  void lines(const Vertex *coords, size_t count);

private:

  void initMatrices();
  void createDefaultTexture();

  GLuint mDefaultTexture;

  GLuint mVBO;
  GLuint mIBO;

};

extern GL gl;
}
