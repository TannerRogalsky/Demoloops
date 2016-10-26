/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#include "common/vector.h"

namespace demoloop {

Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
Vector3::Vector3(float x, float y, float z): x(x), y(y), z(z) {}

float Vector3::getLength() const {
  return sqrtf(x * x + y * y + z * z);
}

float Vector3::normalize(float length) {
  const float length_current = getLength();

  if (length_current > 0) {
    (*this) *= length / length_current;
  }

  return length_current;
}

float Vector3::dot(const Vector3 &v) {
  return x * v.x + y * v.y + z * v.z;
}

Vector3 Vector3::cross(const Vector3 &v) const {
  return Vector3(y * v.z - z * v.y,
                 z * v.x - x * v.z,
                 x * v.y - y * v.x);
}

Vector3 Vector3::operator + (const Vector3 &v) const {
  return Vector3(x + v.x, y + v.y, z + v.z);
}

Vector3 Vector3::operator - (const Vector3 &v) const {
  return Vector3(x - v.x, y - v.y, z - v.z);
}
Vector3 Vector3::operator * (float s) const {
  return Vector3(x * s, y * s, z * s);
}
Vector3 Vector3::operator / (float s) const {
  return Vector3(x / s, y / s, z / s);
}
Vector3 Vector3::operator - () const {
  return Vector3(-x, -y, -z);
}
void Vector3::operator += (const Vector3 &v) {
  x += v.x;
  y += v.y;
  z += v.z;
}
void Vector3::operator -= (const Vector3 &v) {
  x -= v.x;
  y -= v.y;
  z -= v.z;
}
void Vector3::operator *= (float s) {
  x *= s;
  y *= s;
  z *= s;
}
void Vector3::operator /= (float s) {
  x /= s;
  y /= s;
  z /= s;
}
float Vector3::operator * (const Vector3 &v) const {
  return x * v.x + y * v.y + z * v.z;
}
bool Vector3::operator == (const Vector3 &v) const {
  return getLength() == v.getLength();
}
bool Vector3::operator < (const Vector3 &v) const {
  return getLength() < v.getLength();
}



Vector2::Vector2()
  : x(0.0f)
  , y(0.0f)
{
}

Vector2::Vector2(float x, float y)
  : x(x)
  , y(y)
{
}

float Vector2::getLength() const
{
  return sqrtf(x*x + y*y);
}

Vector2 Vector2::getNormal() const
{
  return Vector2(-y, x);
}

Vector2 Vector2::getNormal(float scale) const
{
  return Vector2(-y * scale, x * scale);
}

float Vector2::normalize(float length)
{

  float length_current = getLength();

  if (length_current > 0)
    (*this) *= length / length_current;

  return length_current;
}

Vector2 Vector2::operator + (const Vector2 &v) const
{
  return Vector2(x + v.x, y + v.y);
}

Vector2 Vector2::operator - (const Vector2 &v) const
{
  return Vector2(x - v.getX(), y - v.getY());
}

Vector2 Vector2::operator * (float s) const
{
  return Vector2(x*s, y*s);
}

Vector2 Vector2::operator / (float s) const
{
  return Vector2(x/s, y/s);
}

Vector2 Vector2::operator - () const
{
  return Vector2(-x, -y);
}

void Vector2::operator += (const Vector2 &v)
{
  x += v.getX();
  y += v.getY();
}

void Vector2::operator -= (const Vector2 &v)
{
  x -= v.getX();
  y -= v.getY();
}

void Vector2::operator *= (float s)
{
  x *= s;
  y *= s;
}

void Vector2::operator /= (float s)
{
  x /= s;
  y /= s;
}

float Vector2::operator * (const Vector2 &v) const
{
  return x * v.getX() + y * v.getY();
}

float Vector2::operator ^ (const Vector2 &v) const
{
  return x * v.getY() - y * v.getX();
}

bool Vector2::operator == (const Vector2 &v) const
{
  return getLength() == v.getLength();
}

bool Vector2::operator < (const Vector2 &v) const
{
  return getLength() < v.getLength();
}

/**
 * Accessor methods
 **/

float Vector2::getX() const
{
  return x;
}

float Vector2::getY() const
{
  return y;
}

void Vector2::setX(float x)
{
  this->x = x;
}

void Vector2::setY(float y)
{
  this->y = y;
}

}
