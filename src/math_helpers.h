#pragma once

inline float rotationOffset(const int vertex_count) {
  float interval = DEMOLOOP_M_PI * 2 / vertex_count;
  float rotation_offset = DEMOLOOP_M_PI / 2 + interval;
  if (vertex_count % 2 == 0) {
    rotation_offset = rotation_offset - interval / 2;
  }
  return rotation_offset;
}

inline float jmap(const float &t, const float &s0, const float &s1, const float &e0, const float &e1) {
  float v = 0;
  if (s0 != s1) v = (t-s0)/(s1-s0);
  v = e0 + (v*(e1-e0));
  return v;
}

template <
  typename T,
  typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type
> constexpr T mix(T const &a, T const &b, const float &ratio) {
  return a * (1.0f - ratio) + b * ratio;
}
