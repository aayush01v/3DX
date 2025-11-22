#pragma once
#include <cmath>

struct Vector3 {
    float x, y, z;
    
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vector3 operator-(const Vector3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vector3 operator/(float s) const { return {x / s, y / s, z / s}; }
    
    float dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vector3 cross(const Vector3& v) const {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }
    
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 normalized() const { float l = length(); return l > 0 ? *this / l : Vector3(); }
};

struct Quaternion {
    float w, x, y, z;
    
    Quaternion(float w = 1, float x = 0, float y = 0, float z = 0) : w(w), x(x), y(y), z(z) {}
    
    static Quaternion fromAxisAngle(const Vector3& axis, float angle);
    Quaternion operator*(const Quaternion& q) const;
    Vector3 rotate(const Vector3& v) const;
    Quaternion normalized() const;
};

struct Matrix4 {
    float m[16];
    
    Matrix4();
    static Matrix4 identity();
    static Matrix4 perspective(float fov, float aspect, float near, float far);
    static Matrix4 lookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
    static Matrix4 translate(const Vector3& v);
    static Matrix4 rotate(const Quaternion& q);
    static Matrix4 scale(const Vector3& s);
    
    Matrix4 operator*(const Matrix4& other) const;
};
