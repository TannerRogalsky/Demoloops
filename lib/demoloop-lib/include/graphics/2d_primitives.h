#pragma once

#include "graphics/gl.h"

namespace Demoloop {

void ellipse(GL& gl, const float x, const float y, const float radiusX, const float radiusY, int points = 10);
void circle(GL& gl, const float x, const float y, const float radius, int points = 10);
void rectangle(GL& gl, const float x, const float y, const float w, const float h);
void polygon(GL& gl, const float* xCoords, const float* yCoords, uint32_t count);

}
