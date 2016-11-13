#pragma once

#include <set>
#include <vector>
#include "graphics/gl.h"
#include "common/math.h"
#include "glm/glm.hpp"

namespace demoloop {

class Mesh : public Drawable {
public:

  // Mesh();
  // Mesh(std::vector<Vertex> vertices);
  Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  ~Mesh();

  std::set<uint32_t> getIndexedVertices();
  uint32_t getVertexCount();
  std::vector<Vertex> getLines();

  void draw(glm::mat4 modelView) override;
  using Drawable::draw;

  /**
   * Sets the texture used when drawing the Mesh.
   **/
  void setTexture(Texture *texture);

  /**
   * Disables any texture from being used when drawing the Mesh.
   **/
  void setTexture();

  /**
   * Gets the texture used when drawing the Mesh. May return null if no
   * texture is set.
   **/
  Texture *getTexture() const;

  void buffer();

  std::vector<Vertex> mVertices;
  std::vector<uint32_t> mIndices;

private:
  Texture *mTexture;
  GLuint mIBO;
  GLuint mVBO;
};

}
