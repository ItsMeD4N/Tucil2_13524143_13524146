#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include <cmath>
#include <atomic>
#include <mutex>

using namespace std;

struct Vector3 {
    double x, y, z;

    Vector3();
    Vector3(double x, double y, double z);

    Vector3 operator+(const Vector3& o) const;
    Vector3 operator-(const Vector3& o) const;
    Vector3 operator*(double s)         const;

    double dot(const Vector3& o) const;

    Vector3 cross(const Vector3& o) const;
};

using Vertex = Vector3;

struct Triangle {
    Vector3 v0, v1, v2;
};

using Face = Triangle;

struct AABB {
    Vector3 min, max;

    Vector3 center() const;

    Vector3 halfSize() const;
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
