#include <cmath>
#include <algorithm>
#include "graphics/2d_primitives.h"
#include "graphics/3d_primitives.h"
#include "common/math.h"

namespace demoloop {

void ellipse(GL& gl, const float x, const float y, const float radiusX, const float radiusY, int points) {
  if (points <= 0) points = 1;
  const float interval = (DEMOLOOP_M_PI * 2) / points;

  float* xCoords = new float[points];
  float* yCoords = new float[points];
  float phi = 0.0f;
  for (int i = 0; i < points; ++i, phi += interval)
  {
    xCoords[i] = x + radiusX * cosf(phi);
    yCoords[i] = y + radiusY * sinf(phi);
  }

  polygon(gl, xCoords, yCoords, points);
  delete[] xCoords;
  delete[] yCoords;
}

void circle(GL& gl, const float x, const float y, const float radius, int points) {
  ellipse(gl, x, y, radius, radius, points);
}

void rectangle(GL& gl, const float x, const float y, const float w, const float h) {
  const float xCoords[] = {x, x + w, x + w, x};
  const float yCoords[] = {y, y, y + h, y + h};
  polygon(gl, xCoords, yCoords, 4);
}

void triangle(GL& gl, const float x1, const float y1, const float x2, const float y2, const float x3, const float y3) {
  const float xCoords[] = {x1, x2, x3};
  const float yCoords[] = {y1, y2, y3};
  polygon(gl, xCoords, yCoords, 3);
}

void polygon(GL& gl, const float* xCoords, const float* yCoords, uint32_t count) {
  float* zCoords = new float[count];
  std::fill_n(zCoords, count, 1);
  polygon(gl, xCoords, yCoords, zCoords, count);
  delete[] zCoords;
}

void line(GL& gl, const float x1, const float y1, const float x2, const float y2) {
  line(gl, x1, y1, 1, x2, y2, 1);
}

}
