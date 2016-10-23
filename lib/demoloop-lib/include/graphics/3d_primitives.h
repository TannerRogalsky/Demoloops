#pragma once

#include "graphics/gl.h"

namespace Demoloop {

void cube(Vertex vertices[36], const float cx, const float cy, const float cz, const float radius);
void spherePoints(Vertex vertices[12], const float cx, const float cy, const float cz, const float radius);
void sphereTriangles(Vertex vertices[60], Vertex points[12]);

void polygon(GL& gl, const float* xCoords, const float* yCoords, const float* zCoords, uint32_t count);
void line(GL& gl, const float x1, const float y1, const float z1, const float x2, const float y2, const float z2);
}
