#pragma once 

#include "core.h"
#include "Triangle.h"
#include "Tokenizer.h"
#include <string>
#include <iostream>

class Skin {
public:
    // Vertex and triangle data
    std::vector<SkinVertex> vertices;
    std::vector<Triangle> triangles;

    std::vector<glm::mat4> bindingMats;

    // Load skin data from file
    bool loadFromFile(const std::string& filename);

    // Utility methods for skin processing
    void computeNormals(); // Recompute normals for all vertices
};
