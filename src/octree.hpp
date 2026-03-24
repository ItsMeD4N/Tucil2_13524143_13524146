#ifndef OCTREE_HPP
#define OCTREE_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include <future>
#include <cmath>
#include "types.hpp"

using namespace std;

namespace SAT {

bool triangleIntersectsAABB(const Triangle& tri, const AABB& box);

} 

struct OctreeNode {
    AABB bounds;
    unique_ptr<OctreeNode> children[8];
    bool isLeaf = false;
};

AABB getChildAABB(const AABB& parent, int childIndex);

unique_ptr<OctreeNode> buildOctree(
    const AABB& bounds,
    const vector<Triangle>& relevantTriangles,
    int depth, int maxDepth,
    vector<Voxel>& outVoxels,
    VoxelStats& stats);

unique_ptr<OctreeNode> buildOctreeParallel(
    const AABB& bounds,
    const vector<Triangle>& relevantTriangles,
    int depth, int maxDepth,
    vector<Voxel>& outVoxels,
    VoxelStats& stats);

void runVoxelization(
    const vector<Vertex>& vertices,
    const vector<Face>& triangles,
    int maxDepth,
    vector<Voxel>& outVoxels,
    VoxelStats& stats);

#endif
