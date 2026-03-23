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
        cerr << "[ERROR] Penggunaan: " << argv[0] << " <input.obj> <maxDepth>\n";
        cerr << "  Contoh: " << argv[0] << " test/cube.obj 5\n";
        return 1;
    }

    string inputPath = argv[1];
    int maxDepth;

    try {
        maxDepth = stoi(argv[2]);
    } catch (...) {
        cerr << "[ERROR] maxDepth harus berupa bilangan bulat positif.\n";
        return 1;
    }

    if (maxDepth < 1) {
        cerr << "[ERROR] maxDepth minimal 1, diberikan: " << maxDepth << "\n";
        return 1;
    }

    if (!filesystem::exists(inputPath)) {
        cerr << "[ERROR] File input tidak ditemukan: " << inputPath << "\n";
        return 1;
    }

    vector<Vertex> vertices;
    vector<Face> triangles;

    if (!parseObj(inputPath, vertices, triangles)) {
        cerr << "[ERROR] Gagal mem-parsing file: " << inputPath << "\n";
        return 1;
    }

    if (triangles.empty()) {
        cerr << "[ERROR] Tidak ditemukan segitiga dalam file: " << inputPath << "\n";
        return 1;
    }

    VoxelStats stats;
    stats.maxDepth = maxDepth;
    stats.nodesFormedAtDepth.resize(maxDepth + 1, 0);
    stats.nodesPrunedAtDepth.resize(maxDepth + 1, 0);

    cout << "\n========================================\n";
    cout << "  VOXELIZATION - Octree Divide & Conquer\n";
    cout << "========================================\n";
    cout << "Input        : " << inputPath << "\n";
    cout << "Max Depth    : " << maxDepth << "\n";
    cout << "Vertices     : " << vertices.size() << "\n";
    cout << "Triangles    : " << triangles.size() << "\n";
    cout << "----------------------------------------\n";
    cout << "Memproses...\n\n";

    auto startTime = chrono::high_resolution_clock::now();

    vector<Voxel> generatedVoxels;
    runVoxelization(vertices, triangles, maxDepth, generatedVoxels, stats);

    auto endTime = chrono::high_resolution_clock::now();
    chrono::duration<double> executionTime = endTime - startTime;

    string outputPath = "test/output_voxelized.obj";
    if (!exportToObj(outputPath, generatedVoxels)) {
        cerr << "[ERROR] Gagal mengekspor ke file: " << outputPath << "\n";
        return 1;
    }

    cout << "\n========================================\n";
    cout << "          HASIL VOXELISASI\n";
    cout << "========================================\n";
    cout << "Jumlah Voxel       : " << stats.totalVoxels.load() << "\n";
    cout << "Jumlah Vertex Out  : " << stats.totalVertices.load() << "\n";
    cout << "Jumlah Face Out    : " << stats.totalFaces.load() << "\n";
    cout << "----------------------------------------\n";

    cout << "Nodes Terbentuk per Depth:\n";
    for (int i = 0; i <= maxDepth; ++i)
        cout << "  Depth " << i << " : " << stats.nodesFormedAtDepth[i] << "\n";

    cout << "Nodes Dipangkas (Pruned) per Depth:\n";
    for (int i = 0; i <= maxDepth; ++i)
        cout << "  Depth " << i << " : " << stats.nodesPrunedAtDepth[i] << "\n";

    cout << "----------------------------------------\n";
    cout << "Max Depth          : " << maxDepth << "\n";
    cout << "Waktu Eksekusi     : " << fixed << setprecision(6)
         << executionTime.count() << " detik\n";
    cout << "Output File        : " << outputPath << "\n";
    cout << "========================================\n";

    return 0;
}