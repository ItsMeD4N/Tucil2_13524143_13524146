#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include "types.hpp"
#include "parser.hpp"
#include "octree.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Penggunaan: " << argv[0] << " <input.obj> <maxDepth>\n";
        return 1;
    }

    string inputPath = argv[1];
    int maxDepth;

    try {
        maxDepth = stoi(argv[2]);
    } catch (...) {
        cout << "maxDepth harus berupa bilangan bulat positif.\n";
        return 1;
    }

    if (maxDepth < 1) {
        cout << "maxDepth minimal 1\n";
        return 1;
    }

    if (!filesystem::exists(inputPath)) {
        cout << "File input tidak ditemukan\n";
        return 1;
    }

    vector<Vertex> vertices;
    vector<Face> triangles;

    if (!parseObj(inputPath, vertices, triangles)) {
        cout << "Gagal mem-parsing file\n";
        return 1;
    }

    if (triangles.empty()) {
        cout << "Tidak ditemukan segitiga dalam file\n";
        return 1;
    }

    VoxelStats stats;
    stats.maxDepth = maxDepth;
    stats.nodesFormedAtDepth.resize(maxDepth + 1, 0);
    stats.nodesPrunedAtDepth.resize(maxDepth + 1, 0);

    auto startTime = chrono::high_resolution_clock::now();

    vector<Voxel> generatedVoxels;
    runVoxelization(vertices, triangles, maxDepth, generatedVoxels, stats);

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> executionTime = endTime - startTime;

    filesystem::path inPath(inputPath);
    string newFilename = inPath.stem().string() + "-voxelized" + inPath.extension().string();
    filesystem::path outPath = inPath.parent_path() / newFilename;
    string outputPath = outPath.string();

    if (!exportToObj(outputPath, generatedVoxels)) {
        cout << "Gagal mengekspor ke file\n";
        return 1;
    }

    cout << "\nBanyaknya voxel yang terbentuk: " << stats.totalVoxels.load() << "\n";
    cout << "Banyaknya vertex yang terbentuk: " << stats.totalVertices.load() << "\n";
    cout << "Banyaknya faces yang terbentuk: " << stats.totalFaces.load() << "\n";
    
    cout << "Statistik node octree yang terbentuk dalam format\n";
    for (int i = 1; i <= maxDepth; ++i) {
        cout << i << " : " << stats.nodesFormedAtDepth[i] << "\n";
    }
    
    cout << "Statistik node yang tidak perlu ditelusuri dalam format\n";
    for (int i = 1; i <= maxDepth; ++i) {
        cout << i << " : " << stats.nodesPrunedAtDepth[i] << "\n";
    }
    
    cout << "Kedalaman octree: " << maxDepth << "\n";
    cout << "Lama waktu program berjalan: " << fixed << setprecision(6) << executionTime.count() << " detik\n";
    cout << "File .obj disimpan: " << outputPath << "\n";

    return 0;
}