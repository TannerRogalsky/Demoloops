/**
 * Copyright (c) 2006-2016 DEMOLOOP Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef DEMOLOOP_MATH_H
#define DEMOLOOP_MATH_H

#include <climits> // for CHAR_BIT
#include <cstdlib> // for rand() and RAND_MAX
#include <cstdint>

/* Definitions of useful mathematical constants
 * M_E        - e
 * M_LOG2E    - log2(e)
 * M_LOG10E   - log10(e)
 * M_LN2      - ln(2)
 * M_LN10     - ln(10)
 * M_PI       - pi
 * M_PI_2     - pi/2
 * M_PI_4     - pi/4
 * M_1_PI     - 1/pi
 * M_2_PI     - 2/pi
 * M_2_SQRTPI - 2/sqrt(pi)
 * M_SQRT2    - sqrt(2)
 * M_SQRT1_2  - 1/sqrt(2)
 */

#define DEMOLOOP_M_E        2.71828182845904523536
#define DEMOLOOP_M_LOG2E    1.44269504088896340736
#define DEMOLOOP_M_LOG10E   0.434294481903251827651
#define DEMOLOOP_M_LN2      0.693147180559945309417
#define DEMOLOOP_M_LN10     2.30258509299404568402
#define DEMOLOOP_M_PI       3.14159265358979323846
#define DEMOLOOP_M_PI_2     1.57079632679489661923
#define DEMOLOOP_M_PI_4     0.785398163397448309616
#define DEMOLOOP_M_1_PI     0.318309886183790671538
#define DEMOLOOP_M_2_PI     0.636619772367581343076
#define DEMOLOOP_M_2_SQRTPI 1.12837916709551257390
#define DEMOLOOP_M_SQRT2    1.41421356237309504880
#define DEMOLOOP_M_SQRT1_2  0.707106781186547524401
#define DEMOLOOP_M_TORAD  (float)(DEMOLOOP_M_PI/180.0)
#define DEMOLOOP_M_TODEG    (float)(180.0/DEMOLOOP_M_PI)
#define DEMOLOOP_TORAD(x) (float)(x*DEMOLOOP_M_TORAD)
#define DEMOLOOP_TODEG(x) (float)(x*DEMOLOOP_M_TODEG)

namespace demoloop
{

struct Vertex
{
  float x, y, z;
  float s, t;
  uint8_t r, g, b, a;

  Vertex(): Vertex(0, 0, 0, 0, 0, 255, 255, 255, 255) {}
  Vertex(float x, float y, float z) : Vertex(x, y, z, 0, 0, 255, 255, 255, 255) {}
  Vertex(float x, float y, float z, float s, float t, uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
        x(x), y(y), z(z), s(s), t(t), r(r), g(g), b(b), a(a) {}

  bool operator == (const Vertex &v) const {
    return x == v.x && y == v.y && z == v.z;
  }

  bool operator < (const Vertex &v) const {
    return x < v.x && y < v.y && z < v.z;
  }
};

struct Triangle
{
  Triangle() {}
  Triangle(const Vertex &x, const Vertex &y, const Vertex &z)
    : a(x), b(y), c(z)
  {}
  Vertex a, b, c;
};

inline int next_p2(int x)
{
  x += (x == 0);
  x--;
  for (unsigned int i = 1; i < sizeof(int)*CHAR_BIT; i <<= 1) x |= x >> i;
  return ++x;
}

inline float next_p2(float x)
{
  return static_cast<float>(next_p2(static_cast<int>(x)));
}

} // DEMOLOOP

#endif // DEMOLOOP_MATH_H
