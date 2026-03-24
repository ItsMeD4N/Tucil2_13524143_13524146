#include "types.hpp"

using namespace std;

Vector3::Vector3() : x(0), y(0), z(0) {
}

Vector3::Vector3(double x, double y, double z) : x(x), y(y), z(z) {
}

Vector3 Vector3::operator+(const Vector3& o) const {
    return {x + o.x, y + o.y, z + o.z};
}

Vector3 Vector3::operator-(const Vector3& o) const {
    return {x - o.x, y - o.y, z - o.z};
}

Vector3 Vector3::operator*(double s) const {
    return {x * s, y * s, z * s};
}

double Vector3::dot(const Vector3& o) const {
    return x * o.x + y * o.y + z * o.z;
}

Vector3 Vector3::cross(const Vector3& o) const {
    return {
        y * o.z - z * o.y,
        z * o.x - x * o.z,
        x * o.y - y * o.x
    };
}

Vector3 AABB::center() const {
    return {
        (min.x + max.x) * 0.5,
        (min.y + max.y) * 0.5,
        (min.z + max.z) * 0.5
    };
}

Vector3 AABB::halfSize() const {
    return {
        (max.x - min.x) * 0.5,
        (max.y - min.y) * 0.5,
        (max.z - min.z) * 0.5
    };
}
