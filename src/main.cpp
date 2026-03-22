#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include "types.hpp"
#include "parser.hpp"
#include "octree.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        return 1;
    }

    string inputPath = argv[1];
    int maxDepth;
    
    try {
        maxDepth = stoi(argv[2]);
    } catch (...) {
        return 1;
    }

    vector<Vertex> vertices;
    vector<Face> faces;
    
    VoxelStats stats = {};
    stats.maxDepth = maxDepth;
    stats.nodesFormedAtDepth.resize(maxDepth + 1, 0);
    stats.nodesPrunedAtDepth.resize(maxDepth + 1, 0);

    if (!parseObj(inputPath, vertices, faces)) {
        return 1;
    }

    auto startTime = chrono::high_resolution_clock::now();

    vector<Voxel> generatedVoxels;
    runVoxelization(vertices, faces, maxDepth, generatedVoxels, stats);

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> executionTime = endTime - startTime;

    string outputPath = "test/output_voxelized.obj"; 
    if (!exportToObj(outputPath, generatedVoxels)) {
        return 1;
    }

    cout << stats.totalVoxels << "\n";
    cout << stats.totalVertices << "\n";
    cout << stats.totalFaces << "\n";
    
    for (int i = 1; i <= maxDepth; ++i) {
        cout << i << ": " << stats.nodesFormedAtDepth[i] << "\n";
    }

    for (int i = 1; i <= maxDepth; ++i) {
        cout << i << ": " << stats.nodesPrunedAtDepth[i] << "\n";
    }

    cout << maxDepth << "\n";
    cout << executionTime.count() << "\n";
    cout << outputPath << "\n";

    return 0;
}