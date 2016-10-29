#pragma once

#include <set>
#include <vector>
#include "graphics/gl.h"
#include "common/math.h"

namespace demoloop {

class Mesh {
public:

  // Mesh();
  // Mesh(std::vector<Vertex> vertices);
  Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  ~Mesh();

  std::set<uint32_t> getIndexedVertices();
  uint32_t getVertexCount();
  std::vector<Vertex> getLines();
  void draw();

  std::vector<Vertex> mVertices;
  std::vector<uint32_t> mIndices;

private:

};

}
