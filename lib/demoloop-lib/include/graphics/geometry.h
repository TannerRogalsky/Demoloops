#pragma once

#include <set>
#include <vector>
#include "graphics/gl.h"

namespace demoloop {

class Geometry : public Drawable {
public:
  Geometry();
  ~Geometry();

private:
  std::vector<Vertex> vertices;
  // std::vector<Face> faces;
}

}
