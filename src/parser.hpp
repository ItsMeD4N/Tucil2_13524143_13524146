#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include "types.hpp"

inline int extractVertexIndex(const std::string& token, int totalVertices) {
    std::string indexStr = token.substr(0, token.find('/'));
    int idx = std::stoi(indexStr);

    if (idx < 0) {
        idx = totalVertices + idx;
    } else {
        idx -= 1;
    }
    return idx;
}

inline bool parseObj(const std::string& path,
                     std::vector<Vertex>& vertices,
                     std::vector<Face>& triangles)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Gagal membuka file input: " << path << "\n";
        return false;
    }

    std::vector<Vector3> rawVertices;
    struct RawFace { int i0, i1, i2; };
    std::vector<RawFace> rawFaces;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            double x, y, z;
            if (iss >> x >> y >> z) {
                rawVertices.push_back({x, y, z});
            }
        } else if (prefix == "f") {
            std::string t0, t1, t2;
            if (iss >> t0 >> t1 >> t2) {
                try {
                    int totalV = static_cast<int>(rawVertices.size());
                    int i0 = extractVertexIndex(t0, totalV);
                    int i1 = extractVertexIndex(t1, totalV);
                    int i2 = extractVertexIndex(t2, totalV);

                    if (i0 >= 0 && i0 < totalV &&
                        i1 >= 0 && i1 < totalV &&
                        i2 >= 0 && i2 < totalV) {
                        rawFaces.push_back({i0, i1, i2});
                    }
                } catch (...) {}
            }
        }
    }

    file.close();
    vertices = rawVertices;

    triangles.reserve(rawFaces.size());
    for (const auto& rf : rawFaces) {
        Triangle tri;
        tri.v0 = rawVertices[rf.i0];
        tri.v1 = rawVertices[rf.i1];
        tri.v2 = rawVertices[rf.i2];
        triangles.push_back(tri);
    }

    std::cout << "[INFO] Parsing selesai: "
              << vertices.size() << " vertices, "
              << triangles.size() << " triangles.\n";
    return true;
}

inline bool exportToObj(const std::string& path,
                        const std::vector<Voxel>& voxels)
{
    std::filesystem::path filePath(path);
    if (filePath.has_parent_path()) {
        std::filesystem::create_directories(filePath.parent_path());
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Gagal membuka file output: " << path << "\n";
        return false;
    }

    file << "# Voxelized OBJ output\n";
    file << "# Total voxels: " << voxels.size() << "\n\n";

    static const int faceIndices[12][3] = {
        {4, 5, 6}, {4, 6, 7},   // depan
        {1, 0, 3}, {1, 3, 2},   // belakang
        {5, 1, 2}, {5, 2, 6},   // kanan
        {0, 4, 7}, {0, 7, 3},   // kiri
        {7, 6, 2}, {7, 2, 3},   // atas
        {0, 1, 5}, {0, 5, 4}    // bawah
    };

    for (size_t v = 0; v < voxels.size(); ++v) {
        const AABB& box = voxels[v];
        double x0 = box.min.x, y0 = box.min.y, z0 = box.min.z;
        double x1 = box.max.x, y1 = box.max.y, z1 = box.max.z;

        // 8 vertex kubus
        file << "v " << x0 << " " << y0 << " " << z0 << "\n";
        file << "v " << x1 << " " << y0 << " " << z0 << "\n";
        file << "v " << x1 << " " << y1 << " " << z0 << "\n";
        file << "v " << x0 << " " << y1 << " " << z0 << "\n";
        file << "v " << x0 << " " << y0 << " " << z1 << "\n";
        file << "v " << x1 << " " << y0 << " " << z1 << "\n";
        file << "v " << x1 << " " << y1 << " " << z1 << "\n";
        file << "v " << x0 << " " << y1 << " " << z1 << "\n";

        int offset = static_cast<int>(v) * 8 + 1;

        for (int f = 0; f < 12; ++f) {
            file << "f "
                 << (faceIndices[f][0] + offset) << " "
                 << (faceIndices[f][1] + offset) << " "
                 << (faceIndices[f][2] + offset) << "\n";
        }
        file << "\n";
    }

    file.close();
    std::cout << "[INFO] Ekspor selesai: " << voxels.size() << " voxels -> " << path << "\n";
    return true;
}

#endif
