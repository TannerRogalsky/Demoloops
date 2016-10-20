#pragma once

#include "graphics/gl.h"

namespace Demoloop {

void cube(Vertex vertices[36], const float cx, const float cy, const float cz, const float radius);
void sphere(Vertex vertices[12], const float cx, const float cy, const float cz, const float radius);

void polygon(GL& gl, const float* xCoords, const float* yCoords, const float* zCoords, uint32_t count);

}
