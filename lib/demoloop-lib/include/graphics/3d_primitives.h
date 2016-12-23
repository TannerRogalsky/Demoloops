#pragma once

#include "graphics/gl.h"
#include "graphics/mesh.h"
#include <functional>

namespace demoloop {

Mesh parametric(std::function<Vertex(float, float)> func, const uint32_t slices, const uint32_t stacks);
Mesh cube(const float cx, const float cy, const float cz, const float radius);
Mesh sphere(const float radius, const uint32_t heightSegments = 30, const uint32_t widthSegments = 30);
Mesh icosahedron(const float cx, const float cy, const float cz, const float radius);

void polygon(GL& gl, const float* xCoords, const float* yCoords, const float* zCoords, uint32_t count);
void line(GL& gl, const float x1, const float y1, const float z1, const float x2, const float y2, const float z2);
}
