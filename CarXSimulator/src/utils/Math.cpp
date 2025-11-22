#include "Math.h"

Quaternion Quaternion::fromAxisAngle(const Vector3& axis, float angle) {
    float halfAngle = angle * 0.5f;
    float s = std::sin(halfAngle);
    return Quaternion(std::cos(halfAngle), axis.x * s, axis.y * s, axis.z * s);
}

Quaternion Quaternion::operator*(const Quaternion& q) const {
    return Quaternion(
        w * q.w - x * q.x - y * q.y - z * q.z,
        w * q.x + x * q.w + y * q.z - z * q.y,
        w * q.y - x * q.z + y * q.w + z * q.x,
        w * q.z + x * q.y - y * q.x + z * q.w
    );
}

Vector3 Quaternion::rotate(const Vector3& v) const {
    Quaternion p(0, v.x, v.y, v.z);
    Quaternion qInv(w, -x, -y, -z);
    Quaternion result = (*this) * p * qInv;
    return Vector3(result.x, result.y, result.z);
}

Quaternion Quaternion::normalized() const {
    float len = std::sqrt(w * w + x * x + y * y + z * z);
    return len > 0 ? Quaternion(w / len, x / len, y / len, z / len) : Quaternion();
}

Matrix4::Matrix4() {
    for (int i = 0; i < 16; ++i) m[i] = 0;
}

Matrix4 Matrix4::identity() {
    Matrix4 mat;
    mat.m[0] = mat.m[5] = mat.m[10] = mat.m[15] = 1.0f;
    return mat;
}

Matrix4 Matrix4::perspective(float fov, float aspect, float near, float far) {
    Matrix4 mat;
    float tanHalfFov = std::tan(fov * 0.5f);
    
    mat.m[0] = 1.0f / (aspect * tanHalfFov);
    mat.m[5] = 1.0f / tanHalfFov;
    mat.m[10] = -(far + near) / (far - near);
    mat.m[11] = -1.0f;
    mat.m[14] = -(2.0f * far * near) / (far - near);
    
    return mat;
}

Matrix4 Matrix4::lookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
    Vector3 f = (center - eye).normalized();
    Vector3 s = f.cross(up).normalized();
    Vector3 u = s.cross(f);
    
    Matrix4 mat = identity();
    mat.m[0] = s.x;
    mat.m[4] = s.y;
    mat.m[8] = s.z;
    mat.m[1] = u.x;
    mat.m[5] = u.y;
    mat.m[9] = u.z;
    mat.m[2] = -f.x;
    mat.m[6] = -f.y;
    mat.m[10] = -f.z;
    mat.m[12] = -s.dot(eye);
    mat.m[13] = -u.dot(eye);
    mat.m[14] = f.dot(eye);
    
    return mat;
}
