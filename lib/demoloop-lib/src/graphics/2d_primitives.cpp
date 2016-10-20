#include <cmath>
#include "graphics/2d_primitives.h"
#include "common/math.h"

namespace Demoloop {

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
  const float x = xCoords[0];
  const float y = yCoords[0];

  Vertex *vertices = new Vertex[(count - 2) * 3];
  uint32_t vertexIndex = 0;
  for (uint32_t i = 1; i < count - 1; i++) {
    vertices[vertexIndex].x = x;
    vertices[vertexIndex].y = y;
    vertices[vertexIndex].z = 1;
    vertexIndex++;

    vertices[vertexIndex].x = xCoords[i];
    vertices[vertexIndex].y = yCoords[i];
    vertices[vertexIndex].z = 1;
    vertexIndex++;

    vertices[vertexIndex].x = xCoords[i+1];
    vertices[vertexIndex].y = yCoords[i+1];
    vertices[vertexIndex].z = 1;
    vertexIndex++;
  }

  gl.triangles(vertices, vertexIndex);
}

void line(GL& gl, const float x1, const float y1, const float x2, const float y2) {
  Vertex vertices[2];
  vertices[0].x = x1;
  vertices[0].y = y1;
  vertices[0].z = 1;

  vertices[1].x = x2;
  vertices[1].y = y2;
  vertices[1].z = 1;
  gl.lines(vertices, 2);
}

}
