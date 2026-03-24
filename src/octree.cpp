#include "octree.hpp"
#include <iostream>

using namespace std;

namespace SAT {

bool triangleIntersectsAABB(const Triangle& tri, const AABB& box) {
    Vector3 c = box.center();
    Vector3 h = box.halfSize();

    Vector3 v0 = tri.v0 - c;
    Vector3 v1 = tri.v1 - c;
    Vector3 v2 = tri.v2 - c;

    Vector3 e0 = v1 - v0;
    Vector3 e1 = v2 - v1;
    Vector3 e2 = v0 - v2;

    {
        double triMin = min({v0.x, v1.x, v2.x});
        double triMax = max({v0.x, v1.x, v2.x});
        if (triMax < -h.x || triMin > h.x) {
            return false;
        }

        triMin = min({v0.y, v1.y, v2.y});
        triMax = max({v0.y, v1.y, v2.y});
        if (triMax < -h.y || triMin > h.y) {
            return false;
        }

        triMin = min({v0.z, v1.z, v2.z});
        triMax = max({v0.z, v1.z, v2.z});
        if (triMax < -h.z || triMin > h.z) {
            return false;
        }
    }

    {
        Vector3 normal = e0.cross(e1);
        double d = normal.dot(v0);
        double r = h.x * abs(normal.x) +
                   h.y * abs(normal.y) +
                   h.z * abs(normal.z);
        if (d > r || d < -r) {
            return false;
        }
    }

    auto testAxis = [&](const Vector3& axis) -> bool {
        double p0 = axis.dot(v0);
        double p1 = axis.dot(v1);
        double p2 = axis.dot(v2);
        double triMin = min({p0, p1, p2});
        double triMax = max({p0, p1, p2});
        double r = h.x * abs(axis.x) +
                   h.y * abs(axis.y) +
                   h.z * abs(axis.z);
        return !(triMax < -r || triMin > r);
    };

    Vector3 axes[9] = {
        {0, -e0.z, e0.y}, {e0.z, 0, -e0.x}, {-e0.y, e0.x, 0},
        {0, -e1.z, e1.y}, {e1.z, 0, -e1.x}, {-e1.y, e1.x, 0},
        {0, -e2.z, e2.y}, {e2.z, 0, -e2.x}, {-e2.y, e2.x, 0}
    };

    for (int i = 0; i < 9; ++i) {
        double len2 = axes[i].x * axes[i].x +
                      axes[i].y * axes[i].y +
                      axes[i].z * axes[i].z;
        if (len2 < 1e-12) {
            continue;
        }
        if (!testAxis(axes[i])) {
            return false;
        }
    }

    return true;
}

} 

AABB getChildAABB(const AABB& parent, int childIndex) {
    Vector3 mid = parent.center();
    AABB child;
    child.min.x = (childIndex & 1) ? mid.x : parent.min.x;
    child.max.x = (childIndex & 1) ? parent.max.x : mid.x;

    child.min.y = (childIndex & 2) ? mid.y : parent.min.y;
    child.max.y = (childIndex & 2) ? parent.max.y : mid.y;

    child.min.z = (childIndex & 4) ? mid.z : parent.min.z;
    child.max.z = (childIndex & 4) ? parent.max.z : mid.z;
    return child;
}

unique_ptr<OctreeNode> buildOctree(
    const AABB& bounds,
    const vector<Triangle>& relevantTriangles,
    int depth, int maxDepth,
    vector<Voxel>& outVoxels,
    VoxelStats& stats)
{
    {
        lock_guard<mutex> lock(stats.statsMutex);
        stats.nodesFormedAtDepth[depth]++;
    }

    vector<Triangle> intersecting;
    intersecting.reserve(relevantTriangles.size() / 2);
    for (const auto& tri : relevantTriangles) {
        if (SAT::triangleIntersectsAABB(tri, bounds))
            intersecting.push_back(tri);
    }

    if (intersecting.empty()) {
        lock_guard<mutex> lock(stats.statsMutex);
        stats.nodesPrunedAtDepth[depth]++;
        return nullptr;
    }

    auto node = make_unique<OctreeNode>();
    node->bounds = bounds;

    if (depth == maxDepth) {
        node->isLeaf = true;
        {
            lock_guard<mutex> lock(stats.statsMutex);
            outVoxels.push_back(bounds);
        }
        stats.totalVoxels.fetch_add(1, memory_order_relaxed);
        return node;
    }

    for (int i = 0; i < 8; ++i) {
        AABB childBounds = getChildAABB(bounds, i);
        node->children[i] = buildOctree(
            childBounds, intersecting, depth + 1, maxDepth,
            outVoxels, stats);
    }

    return node;
}

unique_ptr<OctreeNode> buildOctreeParallel(
    const AABB& bounds,
    const vector<Triangle>& relevantTriangles,
    int depth, int maxDepth,
    vector<Voxel>& outVoxels,
    VoxelStats& stats)
{
    {
        lock_guard<mutex> lock(stats.statsMutex);
        stats.nodesFormedAtDepth[depth]++;
    }

    vector<Triangle> intersecting;
    intersecting.reserve(relevantTriangles.size());
    for (const auto& tri : relevantTriangles) {
        if (SAT::triangleIntersectsAABB(tri, bounds))
            intersecting.push_back(tri);
    }

    if (intersecting.empty()) {
        lock_guard<mutex> lock(stats.statsMutex);
        stats.nodesPrunedAtDepth[depth]++;
        return nullptr;
    }

    auto node = make_unique<OctreeNode>();
    node->bounds = bounds;

    if (depth == maxDepth) {
        node->isLeaf = true;
        {
            lock_guard<mutex> lock(stats.statsMutex);
            outVoxels.push_back(bounds);
        }
        stats.totalVoxels.fetch_add(1, memory_order_relaxed);
        return node;
    }

    struct ChildResult {
        unique_ptr<OctreeNode> node;
        vector<Voxel> localVoxels;
    };

    future<ChildResult> futures[8];

    for (int i = 0; i < 8; ++i) {
        AABB childBounds = getChildAABB(bounds, i);
        futures[i] = async(launch::async,
            [childBounds, &intersecting, depth, maxDepth, &stats]() -> ChildResult {
                ChildResult result;
                result.node = buildOctree(
                    childBounds, intersecting, depth + 1, maxDepth,
                    result.localVoxels, stats);
                return result;
            });
    }

    for (int i = 0; i < 8; ++i) {
        ChildResult result = futures[i].get();
        node->children[i] = move(result.node);
        if (!result.localVoxels.empty()) {
            outVoxels.insert(outVoxels.end(),
                result.localVoxels.begin(), result.localVoxels.end());
        }
    }

    return node;
}

void runVoxelization(
    const vector<Vertex>& vertices,
    const vector<Face>& triangles,
    int maxDepth,
    vector<Voxel>& outVoxels,
    VoxelStats& stats)
{
    if (vertices.empty() || triangles.empty()) {
        cerr << "[WARNING] Tidak ada vertex/triangle untuk diproses.\n";
        return;
    }

    Vector3 globalMin = vertices[0];
    Vector3 globalMax = vertices[0];
    for (const auto& v : vertices) {
        globalMin.x = min(globalMin.x, v.x);
        globalMin.y = min(globalMin.y, v.y);
        globalMin.z = min(globalMin.z, v.z);
        globalMax.x = max(globalMax.x, v.x);
        globalMax.y = max(globalMax.y, v.y);
        globalMax.z = max(globalMax.z, v.z);
    }

    double sizeX = globalMax.x - globalMin.x;
    double sizeY = globalMax.y - globalMin.y;
    double sizeZ = globalMax.z - globalMin.z;
    double maxSide = max({sizeX, sizeY, sizeZ}) * 1.01;

    Vector3 center = {
        (globalMin.x + globalMax.x) * 0.5,
        (globalMin.y + globalMax.y) * 0.5,
        (globalMin.z + globalMax.z) * 0.5
    };

    AABB rootBounds;
    rootBounds.min = {center.x - maxSide / 2, center.y - maxSide / 2, center.z - maxSide / 2};
    rootBounds.max = {center.x + maxSide / 2, center.y + maxSide / 2, center.z + maxSide / 2};

    auto root = buildOctreeParallel(rootBounds, triangles, 0, maxDepth, outVoxels, stats);

    int numVoxels = stats.totalVoxels.load();
    stats.totalVertices.store(numVoxels * 8);
    stats.totalFaces.store(numVoxels * 12);
}
