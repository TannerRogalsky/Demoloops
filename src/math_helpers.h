#pragma once

const float PI = 3.1459;

float rotationOffset(const int vertex_count) {
  float interval = PI * 2 / vertex_count;
  float rotation_offset = PI / 2 + interval;
  if (vertex_count % 2 == 0) {
    rotation_offset = rotation_offset - interval / 2;
  }
  return rotation_offset;
}
