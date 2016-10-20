#pragma once

#include "graphics/gl.h"

namespace Demoloop {

void ellipse(GL& gl, const float x, const float y, const float radiusX, const float radiusY, int points = 10);
void circle(GL& gl, const float x, const float y, const float radius, int points = 10);
void rectangle(GL& gl, const float x, const float y, const float w, const float h);
void triangle(GL& gl, const float x1, const float y1, const float x2, const float y2, const float x3, const float y3);
void polygon(GL& gl, const float* xCoords, const float* yCoords, uint32_t count);

void line(GL& gl, const float x1, const float y1, const float x2, const float y2);

}
