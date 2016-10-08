#pragma once

struct RGB {
  uint8_t r, g, b;
};

float hue2rgb(float p, float q, float t);

RGB hsl2rgb(float hue, float saturation, float lightness);

uint32_t rgb2uint32(const RGB rgb);
