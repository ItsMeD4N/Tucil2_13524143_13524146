#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include <cmath>
#include <atomic>
#include <mutex>

struct Vector3 {
    double x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vector3 operator-(const Vector3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vector3 operator*(double s)         const { return {x * s,   y * s,   z * s  }; }

    double dot(const Vector3& o) const { return x * o.x + y * o.y + z * o.z; }

    Vector3 cross(const Vector3& o) const {
        return {
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        };
    }
};

using Vertex = Vector3;

struct Triangle {
    Vector3 v0, v1, v2;
};

using Face = Triangle;

struct AABB {
    Vector3 min, max;

    Vector3 center() const {
        return {(min.x + max.x) * 0.5, (min.y + max.y) * 0.5, (min.z + max.z) * 0.5};
    }

    Vector3 halfSize() const {
        return {(max.x - min.x) * 0.5, (max.y - min.y) * 0.5, (max.z - min.z) * 0.5};
    }
};

using Voxel = AABB;

struct VoxelStats {
    std::atomic<int> totalVoxels{0};
    std::atomic<int> totalVertices{0};
    std::atomic<int> totalFaces{0};
    int maxDepth = 0;

    std::vector<int> nodesFormedAtDepth;
    std::vector<int> nodesPrunedAtDepth;
    std::mutex statsMutex;
};

#endif
