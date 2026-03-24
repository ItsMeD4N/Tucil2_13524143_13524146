#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include "types.hpp"

using namespace std;

int extractVertexIndex(const string& token, int totalVertices);

bool parseObj(const string& path,
             vector<Vertex>& vertices,
             vector<Face>& triangles);

bool exportToObj(const string& path,
                const vector<Voxel>& voxels);

#endif
