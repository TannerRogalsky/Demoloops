#include "graphics/3d_primitives.h"
#include <cmath>
#include <numeric>
#include <vector>

namespace demoloop {

Mesh parametric(std::function<Vertex(float, float)> func, const uint32_t slices, const uint32_t stacks) {
  std::vector<Vertex> vertices;
  vertices.reserve(stacks * slices);
  const uint32_t sliceCount = slices + 1;

  for (uint32_t i = 0; i <= stacks; ++i) {
    const float v = static_cast<float>(i) / stacks;

    for (uint32_t j = 0; j <= slices; ++j) {
      const float u = static_cast<float>(j) / slices;

      vertices.push_back(func(u, v));
    }
  }

  std::vector<uint32_t> indices;
  indices.reserve(stacks * slices * 6);
  for (uint32_t i = 0; i < stacks; ++i) {
    for (uint32_t j = 0; j < slices; ++j) {

      const float a = i * sliceCount + j;
      const float b = i * sliceCount + j + 1;
      const float c = ( i + 1 ) * sliceCount + j + 1;
      const float d = ( i + 1 ) * sliceCount + j;

      // faces one and two
      indices.push_back(a);
      indices.push_back(b);
      indices.push_back(d);

      indices.push_back(b);
      indices.push_back(c);
      indices.push_back(d);
    }
  }

  return Mesh(vertices, indices);
}

Mesh plane(const float width, const float height, const uint32_t slices, const uint32_t stacks) {
  return parametric([&width, &height](const float u, const float v) {
    return Vertex(
      (u - 0.5) * width, (v - 0.5) * height, 0,
      u, v,
      255, 255, 255, 255
    );
   }, slices, stacks);
}

Mesh cube(const float cx, const float cy, const float cz, const float radius) {
  static const GLfloat g_vertex_buffer_data[] = {
    // Top face
     1.0f, 1.0f, -1.0f , 1.0f, 0.0f,  // Top-right of top face
    -1.0f, 1.0f, -1.0f , 0.0f, 0.0f,  // Top-left of top face
    -1.0f, 1.0f,  1.0f , 0.0f, 1.0f,  // Bottom-left of top face
     1.0f, 1.0f,  1.0f , 1.0f, 1.0f,  // Bottom-right of top face

    // Bottom face
     1.0f, -1.0f, -1.0f, 1.0f, 0.0f,  // Top-right of bottom face
    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,  // Top-left of bottom face
    -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  // Bottom-left of bottom face
     1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  // Bottom-right of bottom face

    // Front face
     1.0f,  1.0f, 1.0f , 1.0f, 0.0f,  // Top-Right of front face
    -1.0f,  1.0f, 1.0f , 0.0f, 0.0f,  // Top-left of front face
    -1.0f, -1.0f, 1.0f , 0.0f, 1.0f,  // Bottom-left of front face
     1.0f, -1.0f, 1.0f , 1.0f, 1.0f,  // Bottom-right of front face

    // Back face
    -1.0f,  1.0f, -1.0f, 1.0f, 0.0f,  // Top-Right of back face
     1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  // Top-Left of back face
     1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  // Bottom-Left of back face
    -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  // Bottom-Right of back face

    // Left face
    -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,  // Top-Right of left face
    -1.0f,  1.0f, -1.0f, 0.0f, 0.0f,  // Top-Left of left face
    -1.0f, -1.0f, -1.0f, 0.0f, 1.0f,  // Bottom-Left of left face
    -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  // Bottom-Right of left face

    // Right face
     1.0f,  1.0f,  1.0f, 0.0f, 0.0f,  // Top-Left of left face
     1.0f,  1.0f, -1.0f, 1.0f, 0.0f,  // Top-Right of left face
     1.0f, -1.0f, -1.0f, 1.0f, 1.0f,  // Bottom-Right of left face
     1.0f, -1.0f,  1.0f, 0.0f, 1.0f,  // Bottom-Left of left face
  };

  std::vector<uint32_t> indices = {
    0+0, 1+0, 2+0,
    0+0, 2+0, 3+0,

    2+4, 1+4, 0+4,
    3+4, 2+4, 0+4,

    0+8, 1+8, 2+8,
    0+8, 2+8, 3+8,

    0+12, 1+12, 2+12,
    0+12, 2+12, 3+12,

    0+16, 1+16, 2+16,
    0+16, 2+16, 3+16,

    2+20, 1+20, 0+20,
    3+20, 2+20, 0+20
  };

  std::vector<Vertex> vertices;
  vertices.reserve(24);
  for (int i = 0; i < 24; ++i)
  {
    Vertex v;
    v.x = g_vertex_buffer_data[i * 5 + 0] * radius + cx;
    v.y = g_vertex_buffer_data[i * 5 + 1] * radius + cy;
    v.z = g_vertex_buffer_data[i * 5 + 2] * radius + cz;
    v.s = g_vertex_buffer_data[i * 5 + 3];
    v.t = g_vertex_buffer_data[i * 5 + 4];
    vertices.push_back(v);
  }

  return Mesh(vertices, indices);
}

Mesh sphere(const float radius, const uint32_t heightSegments, const uint32_t widthSegments) {
  const float phiStart = 0;
  const float phiLength = DEMOLOOP_M_PI * 2;

  const float thetaStart = 0;
  const float thetaLength = DEMOLOOP_M_PI;

  const float thetaEnd = thetaStart + thetaLength;
  const uint32_t vertexCount = ((widthSegments + 1) * (heightSegments + 1));

  std::vector<Vertex> vertices;
  vertices.reserve(vertexCount);
  for ( uint32_t y = 0; y <= heightSegments; ++y ) {
    const float v = static_cast<float>(y) / heightSegments;

    for ( uint32_t x = 0; x <= widthSegments; ++x ) {
      const float u = static_cast<float>(x) / widthSegments;

      const float phi = phiStart + u * phiLength;
      const float theta = thetaStart + v * thetaLength;

      const float px = -radius * cosf( phi ) * sinf( theta );
      const float py = radius * cosf( theta );
      const float pz = radius * sinf( phi ) * sinf( theta );

      vertices.push_back({px, py, pz, u, v});
    }
  }

  std::vector<uint32_t> indices;
  for ( uint32_t y = 0; y < heightSegments; ++y ) {
    for ( uint32_t x = 0; x < widthSegments; ++x ) {

      const uint32_t v1 = (y) * (widthSegments + 1) + x + 1;
      const uint32_t v2 = (y) * (widthSegments + 1) + x;
      const uint32_t v3 = (y + 1) * (widthSegments + 1) + x;
      const uint32_t v4 = (y + 1) * (widthSegments + 1) + x + 1;

      if ( y != 0 || thetaStart > 0 ) {
        indices.push_back(v1);
        indices.push_back(v2);
        indices.push_back(v4);
      }
      if ( y != heightSegments - 1 || thetaEnd < DEMOLOOP_M_PI ) {
        indices.push_back(v2);
        indices.push_back(v3);
        indices.push_back(v4);
      }

    }
  }

  return Mesh(vertices, indices);
}

Mesh icosahedron(const float cx, const float cy, const float cz, const float radius) {
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

  return Mesh(vertices, indices);
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

  gl.bindTexture(gl.getDefaultTexture());
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

  gl.bindTexture(gl.getDefaultTexture());
  gl.lines(vertices, 2);
}

}
