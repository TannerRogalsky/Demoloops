#include "graphics/3d_primitives.h"
#include <cmath>

namespace Demoloop {

void cube(Vertex vertices[36], const float cx, const float cy, const float cz, const float radius) {
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

  // Vertex vertices[36];
  for (int i = 0; i < 36; ++i) {
    vertices[i].x = g_vertex_buffer_data[i * 3 + 0] * radius + cx;
    vertices[i].y = g_vertex_buffer_data[i * 3 + 1] * radius + cy;
    vertices[i].z = g_vertex_buffer_data[i * 3 + 2] * radius + cz;
  }
  // return vertices;
}

void sphere(Vertex vertices[60], const float cx, const float cy, const float cz, const float radius) {
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

  vertices[0] = points[0];
  vertices[1] = points[11];
  vertices[2] = points[5];

  vertices[3] = points[0];
  vertices[4] = points[5];
  vertices[5] = points[1];

  vertices[6] = points[0];
  vertices[7] = points[1];
  vertices[8] = points[7];

  vertices[9] = points[0];
  vertices[10] = points[7];
  vertices[11] = points[10];

  vertices[12] = points[0];
  vertices[13] = points[10];
  vertices[14] = points[11];

  vertices[15] = points[1];
  vertices[16] = points[5];
  vertices[17] = points[9];

  vertices[18] = points[5];
  vertices[19] = points[11];
  vertices[20] = points[4];

  vertices[21] = points[11];
  vertices[22] = points[10];
  vertices[23] = points[2];

  vertices[24] = points[10];
  vertices[25] = points[7];
  vertices[26] = points[6];

  vertices[27] = points[7];
  vertices[28] = points[1];
  vertices[29] = points[8];

  vertices[30] = points[3];
  vertices[31] = points[9];
  vertices[32] = points[4];

  vertices[33] = points[3];
  vertices[34] = points[4];
  vertices[35] = points[2];

  vertices[36] = points[3];
  vertices[37] = points[2];
  vertices[38] = points[6];

  vertices[39] = points[3];
  vertices[40] = points[6];
  vertices[41] = points[8];

  vertices[42] = points[3];
  vertices[43] = points[8];
  vertices[44] = points[9];

  vertices[45] = points[4];
  vertices[46] = points[9];
  vertices[47] = points[5];

  vertices[48] = points[2];
  vertices[49] = points[4];
  vertices[50] = points[11];

  vertices[51] = points[6];
  vertices[52] = points[2];
  vertices[53] = points[10];

  vertices[54] = points[8];
  vertices[55] = points[6];
  vertices[56] = points[7];

  vertices[57] = points[9];
  vertices[58] = points[8];
  vertices[59] = points[1];

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

}
