#include "graphics/mesh.h"
#include <algorithm>
#include <iostream>

namespace demoloop {
  // Mesh::Mesh() : Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {}
  // Mesh::Mesh(std::vector<Vertex> vertices) : Mesh(vertices, std::vector<uint32_t> indices) {}
  Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : mVertices(vertices), mIndices(indices)
  {
  }

  Mesh::~Mesh() {}

  std::set<uint32_t> Mesh::getIndexedVertices() {
    return std::set<uint32_t>(mIndices.begin(), mIndices.end());
  }

  uint32_t Mesh::getVertexCount() {
    return mVertices.size();
  }

  void Mesh::draw() {
    gl.triangles(&mVertices[0], &mIndices[0], mVertices.size());
  }
}
