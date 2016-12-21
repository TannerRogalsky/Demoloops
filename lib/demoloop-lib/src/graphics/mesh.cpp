#include "graphics/mesh.h"

namespace demoloop {
  // Mesh::Mesh() : Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {}
  // Mesh::Mesh(std::vector<Vertex> vertices) : Mesh(vertices, std::vector<uint32_t> indices) {}
  Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : mVertices(vertices), mIndices(indices), mTexture(nullptr)
  {
    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mIBO);
    buffer();
  }

  Mesh::~Mesh() {
    glDeleteBuffers(1, &mVBO);
    glDeleteBuffers(1, &mIBO);
  }

  std::set<uint32_t> Mesh::getIndexedVertices() {
    return std::set<uint32_t>(mIndices.begin(), mIndices.end());
  }

  uint32_t Mesh::getVertexCount() {
    return mVertices.size();
  }

  void Mesh::buffer() {
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices.data()[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(uint32_t), &mIndices.data()[0], GL_DYNAMIC_DRAW);
  }

  std::vector<Vertex> Mesh::getLines() {
    std::vector<Vertex> lines;
    lines.reserve(mIndices.size() * 2);
    for (uint32_t i = 0; i < mIndices.size() - 2; i+=3) {
      lines.push_back(mVertices[mIndices[i + 0]]);
      lines.push_back(mVertices[mIndices[i + 1]]);
      lines.push_back(mVertices[mIndices[i + 1]]);
      lines.push_back(mVertices[mIndices[i + 2]]);
      lines.push_back(mVertices[mIndices[i + 2]]);
      lines.push_back(mVertices[mIndices[i + 0]]);
    }
    return lines;
  }

  void Mesh::setTexture(Texture *tex)
  {
    mTexture = tex;
  }

  void Mesh::setTexture()
  {
    mTexture = nullptr;
  }

  Texture *Mesh::getTexture() const
  {
    return mTexture;
  }

  void Mesh::draw(glm::mat4 modelView) {
    if (mTexture)
      gl.bindTexture(*(GLuint *) mTexture->getHandle());
    else
      gl.bindTexture(gl.getDefaultTexture());

    gl.prepareDraw(modelView);

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_COLOR | ATTRIBFLAG_TEXCOORD);
    glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, x));
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, s));
    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*) offsetof(Vertex, r));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    gl.drawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, 0);
  }
}
