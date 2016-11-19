#include <stdint.h>
#include <cmath>
#include "hsl.h"
#include <glm/vec4.hpp>

float hue2rgb(float p, float q, float t) {
  if (t < 0) { t += 1; }
  if (t > 1) { t -= 1; }
  if ((6 * t) < 1) { return (p + (q - p) * 6 * t); }
  if ((2 * t) < 1) { return q; }
  if ((3 * t) < 2) { return (p + (q - p) * ((2.0 / 3.0) - t) * 6); }
  return p;
}

RGB hsl2rgb(float h, float s, float l) {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  if (s == 0) {
    r = g = b = (uint8_t) (l * 255);
  } else {
    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;

    r = hue2rgb(p, q, h + 1.0/3.0) * 255;
    g = hue2rgb(p, q, h) * 255;
    b = hue2rgb(p, q, h - 1.0/3.0) * 255;
  }

  return {r, g, b};
}

glm::vec4 hsl2rgbf(float h, float s, float l) {
  float r = 0;
  float g = 0;
  float b = 0;

  if (s == 0) {
    r = g = b = l;
  } else {
    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;

    r = hue2rgb(p, q, h + 1.0/3.0);
    g = hue2rgb(p, q, h);
    b = hue2rgb(p, q, h - 1.0/3.0);
  }

  return glm::vec4(r, g, b, 1.0f);
}

uint32_t rgb2uint32(const RGB color) {
  return  (255 << 24) +
          (color.r << 16) +
          (color.g << 8) +
          color.b;
}
