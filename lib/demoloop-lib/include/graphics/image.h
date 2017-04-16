#pragma once

// #include "graphics/Color.h"
// #include "common/string_map.h"
// #include "common/int.h"
#include "graphics/texture.h"
// #include "graphics/Volatile.h"
#include "graphics/gl.h"
#include <string>

namespace demoloop
{

class Image : public Texture
{
public:
  Image(const std::string &path);
  virtual ~Image();

  // Implements Volatile.
  // virtual bool loadVolatile();
  // virtual void unloadVolatile();

  // Implements Drawable.
  void draw(glm::mat4 modelTransform) override;
  using Drawable::draw;

  // Implements Texture.
  virtual void drawq(Quad *quad, glm::mat4 modelTransform) override;
  virtual void setFilter(const Texture::Filter &f) override;
  virtual bool setWrap(const Texture::Wrap &w) override;
  virtual const void *getHandle() const override;

private:
  int width, height;
  demoloop::Vertex vertices[4];
  GLuint texture;

  void drawv(const glm::mat4 &t, const Vertex *v);

};
}
