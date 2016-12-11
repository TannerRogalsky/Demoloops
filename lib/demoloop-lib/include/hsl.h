#pragma once

#include <glm/fwd.hpp>

struct RGB {
  uint8_t r, g, b;

  RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
};

float hue2rgb(float p, float q, float t);

RGB hsl2rgb(float hue, float saturation, float lightness);
glm::vec4 hsl2rgbf(float hue, float saturation, float lightness);

uint32_t rgb2uint32(const RGB rgb);
