#include "graphics/3d_primitives.h"
#include <cmath>
#include <numeric>

namespace demoloop {

Mesh* cube(const float cx, const float cy, const float cz, const float radius) {
  static const GLfloat g_vertex_buffer_data[] = {
      -1.0f,-1.0f,-1.0f,
      -1.0f,-1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,

      1.0f, 1.0f,-1.0f,
      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f,

      1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f,-1.0f,
      1.0f,-1.0f,-1.0f,

      1.0f, 1.0f,-1.0f,
      1.0f,-1.0f,-1.0f,
      -1.0f,-1.0f,-1.0f,

      -1.0f,-1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,

      1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f, 1.0f,
      -1.0f,-1.0f,-1.0f,

      -1.0f, 1.0f, 1.0f,
      -1.0f,-1.0f, 1.0f,
      1.0f,-1.0f, 1.0f,

      1.0f, 1.0f, 1.0f,
      1.0f,-1.0f,-1.0f,
      1.0f, 1.0f,-1.0f,

      1.0f,-1.0f,-1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f,-1.0f, 1.0f,

      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f,-1.0f,
      -1.0f, 1.0f,-1.0f,

      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f,-1.0f,
      -1.0f, 1.0f, 1.0f,

      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      1.0f,-1.0f, 1.0f
  };

  std::vector<uint32_t> indices = {
    0, 1, 2, 3, 0, 5, 6, 0, 8, 3, 8,
    0, 0, 2, 5, 6, 1, 0, 2, 1, 6, 21,
    8, 3, 8, 21, 6, 21, 3, 5, 21, 5, 2, 21, 2, 6
  };

  std::vector<Vertex> vertices;
  vertices.reserve(36);
  for (int i = 0; i < 36; ++i)
  {
    Vertex v;
    v.x = g_vertex_buffer_data[i * 3 + 0] * radius + cx;
    v.y = g_vertex_buffer_data[i * 3 + 1] * radius + cy;
    v.z = g_vertex_buffer_data[i * 3 + 2] * radius + cz;
    vertices.push_back(v);
  }

  return new Mesh(vertices, indices);
}

Mesh* sphere(const float cx, const float cy, const float cz, const float radius) {
  const float t = (1.0 + sqrt(5.0)) / 2.0 * radius;

  Vertex points[12];
  points[0].x = -radius + cx;
  points[0].y =  t + cy;
  points[0].z =  0 + cz;

  points[1].x =  radius + cx;
  points[1].y =  t + cy;
  points[1].z =  0 + cz;

  points[2].x = -radius + cx;
  points[2].y = -t + cy;
  points[2].z =  0 + cz;

  points[3].x =  radius + cx;
  points[3].y = -t + cy;
  points[3].z =  0 + cz;


  points[4].x =  0 + cx;
  points[4].y = -radius + cy;
  points[4].z =  t + cz;

  points[5].x =  0 + cx;
  points[5].y =  radius + cy;
  points[5].z =  t + cz;

  points[6].x =  0 + cx;
  points[6].y = -radius + cy;
  points[6].z = -t + cz;

  points[7].x =  0 + cx;
  points[7].y =  radius + cy;
  points[7].z = -t + cz;


  points[8].x =  t + cx;
  points[8].y =  0 + cy;
  points[8].z = -radius + cz;

  points[9].x =  t + cx;
  points[9].y =  0 + cy;
  points[9].z =  radius + cz;

  points[10].x = -t + cx;
  points[10].y =  0 + cy;
  points[10].z = -radius + cz;

  points[11].x = -t + cx;
  points[11].y =  0 + cy;
  points[11].z =  radius + cz;

  std::vector<Vertex> vertices;
  vertices.reserve(60);

  vertices.push_back(Vertex(points[0]));
  vertices.push_back(Vertex(points[11]));
  vertices.push_back(Vertex(points[5]));

  vertices.push_back(Vertex(points[0]));
  vertices.push_back(Vertex(points[5]));
  vertices.push_back(Vertex(points[1]));

  vertices.push_back(Vertex(points[0]));
  vertices.push_back(Vertex(points[1]));
  vertices.push_back(Vertex(points[7]));

  vertices.push_back(Vertex(points[0]));
  vertices.push_back(Vertex(points[7]));
  vertices.push_back(Vertex(points[10]));

  vertices.push_back(Vertex(points[0]));
  vertices.push_back(Vertex(points[10]));
  vertices.push_back(Vertex(points[11]));

  vertices.push_back(Vertex(points[1]));
  vertices.push_back(Vertex(points[5]));
  vertices.push_back(Vertex(points[9]));

  vertices.push_back(Vertex(points[5]));
  vertices.push_back(Vertex(points[11]));
  vertices.push_back(Vertex(points[4]));

  vertices.push_back(Vertex(points[11]));
  vertices.push_back(Vertex(points[10]));
  vertices.push_back(Vertex(points[2]));

  vertices.push_back(Vertex(points[10]));
  vertices.push_back(Vertex(points[7]));
  vertices.push_back(Vertex(points[6]));

  vertices.push_back(Vertex(points[7]));
  vertices.push_back(Vertex(points[1]));
  vertices.push_back(Vertex(points[8]));

  vertices.push_back(Vertex(points[3]));
  vertices.push_back(Vertex(points[9]));
  vertices.push_back(Vertex(points[4]));

  vertices.push_back(Vertex(points[3]));
  vertices.push_back(Vertex(points[4]));
  vertices.push_back(Vertex(points[2]));

  vertices.push_back(Vertex(points[3]));
  vertices.push_back(Vertex(points[2]));
  vertices.push_back(Vertex(points[6]));

  vertices.push_back(Vertex(points[3]));
  vertices.push_back(Vertex(points[6]));
  vertices.push_back(Vertex(points[8]));

  vertices.push_back(Vertex(points[3]));
  vertices.push_back(Vertex(points[8]));
  vertices.push_back(Vertex(points[9]));

  vertices.push_back(Vertex(points[4]));
  vertices.push_back(Vertex(points[9]));
  vertices.push_back(Vertex(points[5]));

  vertices.push_back(Vertex(points[2]));
  vertices.push_back(Vertex(points[4]));
  vertices.push_back(Vertex(points[11]));

  vertices.push_back(Vertex(points[6]));
  vertices.push_back(Vertex(points[2]));
  vertices.push_back(Vertex(points[10]));

  vertices.push_back(Vertex(points[8]));
  vertices.push_back(Vertex(points[6]));
  vertices.push_back(Vertex(points[7]));

  vertices.push_back(Vertex(points[9]));
  vertices.push_back(Vertex(points[8]));
  vertices.push_back(Vertex(points[1]));

  std::vector<uint32_t> indices = {
    0, 1, 2, 0, 2, 5, 0, 5, 8, 0, 8, 11, 0,
    11, 1, 5, 2, 17, 2, 1, 20, 1, 11, 23, 11,
    8, 26, 8, 5, 29, 30, 17, 20, 30, 20, 23,
    30, 23, 26, 30, 26, 29, 30, 29, 17, 20,
    17, 2, 23, 20, 1, 26, 23, 11, 29, 26, 8,
    17, 29, 5
  };

  return new Mesh(vertices, indices);
}

void polygon(GL& gl, const float* xCoords, const float* yCoords, const float* zCoords, uint32_t count) {
  const float x = xCoords[0];
  const float y = yCoords[0];
  const float z = zCoords[0];

  Vertex *vertices = new Vertex[(count - 2) * 3];
  uint32_t vertexIndex = 0;
  for (uint32_t i = 1; i < count - 1; i++) {
    vertices[vertexIndex].x = x;
    vertices[vertexIndex].y = y;
    vertices[vertexIndex].z = z;
    vertexIndex++;

    vertices[vertexIndex].x = xCoords[i];
    vertices[vertexIndex].y = yCoords[i];
    vertices[vertexIndex].z = zCoords[i];
    vertexIndex++;

    vertices[vertexIndex].x = xCoords[i+1];
    vertices[vertexIndex].y = yCoords[i+1];
    vertices[vertexIndex].z = zCoords[i+1];
    vertexIndex++;
  }

  gl.triangles(vertices, vertexIndex);
  delete[] vertices;
}

void line(GL& gl, const float x1, const float y1, const float z1, const float x2, const float y2, const float z2) {
  Vertex vertices[2];
  vertices[0].x = x1;
  vertices[0].y = y1;
  vertices[0].z = z1;

  vertices[1].x = x2;
  vertices[1].y = y2;
  vertices[1].z = z2;
  gl.lines(vertices, 2);
}

}
