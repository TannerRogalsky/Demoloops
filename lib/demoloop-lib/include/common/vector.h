#pragma once

// STD
#include <cmath>

namespace demoloop
{

class Vector3
{
public:

  float x, y, z;

  Vector3();
  Vector3(float x, float y, float z);

  float getLength() const;
  float normalize(float length = 1.0);
  float dot(const Vector3 &v);
  Vector3 cross(const Vector3 &v) const;

  Vector3 operator + (const Vector3 &v) const;
  Vector3 operator - (const Vector3 &v) const;
  Vector3 operator * (float s) const;
  Vector3 operator / (float s) const;
  Vector3 operator - () const;
  void operator += (const Vector3 &v);
  void operator -= (const Vector3 &v);
  void operator *= (float s);
  void operator /= (float s);
  float operator * (const Vector3 &v) const;
  float operator ^ (const Vector3 &v) const;
  bool operator == (const Vector3 &v) const;
  bool operator < (const Vector3 &v) const;

};

class Vector2
{
public:

  // The components.
  float x, y;

  /**
   * Creates a new (1,1) Vector.
   **/
  Vector2();

  /**
   * Creates a new Vector.
   * @param x The x position/dimension.
   * @param y The y position/dimension.
   **/
  Vector2(float x, float y);

  /**
   * Gets the length of the Vector.
   * @return The length of the Vector.
   *
   * This method requires sqrtf() and should be used
   * carefully.
   **/
  float getLength() const;

  /**
   * Normalizes the Vector.
   * @param length Desired length of the vector.
   * @return The old length of the Vector.
   **/
  float normalize(float length = 1.0);

  /**
   * Gets a vector perpendicular to the Vector.
   * To get the true (normalized) normal, use v.getNormal(1.0f / v.getLength())
   * @return A normal to the Vector.
   **/
  Vector2 getNormal() const;

  /**
   * Gets a vector perpendicular to the Vector.
   * To get the true (normalized) normal, use v.getNormal(1.0f / v.getLength())
   * @param scale factor to apply.
   * @return A normal to the Vector.
   **/
  Vector2 getNormal(float scale) const;

  /**
   * Adds a Vector to this Vector.
   * @param v The Vector we want to add to this Vector.
   * @return The resulting Vector.
   **/
  Vector2 operator + (const Vector2 &v) const;

  /**
   * Substracts a Vector to this Vector.
   * @param v The Vector we want to subtract to this Vector.
   * @return The resulting Vector.
   **/
  Vector2 operator - (const Vector2 &v) const;

  /**
   * Resizes a Vector by a scalar.
   * @param s The scalar with which to resize the Vector.
   * @return The resulting Vector.
   **/
  Vector2 operator * (float s) const;

  /**
   * Resizes a Vector by a scalar.
   * @param s The scalar with which to resize the Vector.
   * @return The resulting Vector.
   **/
  Vector2 operator / (float s) const;

  /**
   * Reverses the Vector.
   * @return The reversed Vector.
   **/
  Vector2 operator - () const;

  /**
   * Adds a Vector to this Vector, and also saves changes in the first Vector.
   * @param v The Vector we want to add to this Vector.
   **/
  void operator += (const Vector2 &v);

  /**
   * Subtracts a Vector to this Vector, and also saves changes in the first Vector.
   * @param v The Vector we want to subtract to this Vector.
   **/
  void operator -= (const Vector2 &v);

  /**
   * Resizes the Vector, and also saves changes in the first Vector.
   * @param s The scalar by which we want to resize the Vector.
   **/
  void operator *= (float s);

  /**
   * Resizes the Vector, and also saves changes in the first Vector.
   * @param s The scalar by which we want to resize the Vector.
   **/
  void operator /= (float s);

  /**
   * Calculates the dot product of two Vectors.
   * @return The dot product of the two Vectors.
   **/
  float operator * (const Vector2 &v) const;

  /**
   * Calculates the cross product of two Vectors.
   * @return The cross product of the two Vectors.
   **/
  float operator ^ (const Vector2 &v) const;

  bool operator == (const Vector2 &v) const;

  bool operator < (const Vector2 &v) const;
  /**
   * Gets the x value of the Vector.
   * @return The x value of the Vector.
   **/
  float getX() const;

  /**
   * Gets the x value of the Vector.
   * @return The x value of the Vector.
   **/
  float getY() const;

  /**
   * Sets the x value of the Vector.
   * @param x The x value of the Vector.
   **/
  void setX(float x);

  /**
   * Sets the x value of the Vector.
   * @param y The x value of the Vector.
   **/
  void setY(float y);

};

}
