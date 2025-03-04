#pragma once

#include "core.h"
#include <vector>

struct VertexWeight {
    int jointIndex;
    float weight;
    VertexWeight(int jointIndex, float weight)
        : jointIndex(jointIndex), weight(weight) {}
};

class Vertex {
public:
    glm::vec3 position;
    glm::vec3 normal;

    Vertex(const glm::vec3& pos = glm::vec3(0.0f), const glm::vec3& norm = glm::vec3(0.0f))
        : position(pos), normal(norm) {}

    virtual ~Vertex() = default; // Virtual destructor for polymorphism
};

class SkinVertex : public Vertex {
public:
    std::vector<VertexWeight> weights;

    SkinVertex(const glm::vec3& pos = glm::vec3(0.0f), const glm::vec3& norm = glm::vec3(0.0f))
        : Vertex(pos, norm) {}
};

struct GPUSkinVertex {
    glm::vec3 position;
    glm::vec3 normal;
    uint8_t jointIndices[4]; // 4个关节索引（0-150）
    uint8_t weights[3];      // 3个权重（0-150对应0.0-1.0）
    uint8_t validWeights;    // 实际有效的权重数量（）

    GPUSkinVertex(const SkinVertex& src) {
        position = src.position;
        normal = src.normal;
        validWeights = 0;

        std::fill_n(jointIndices, 4, 0xFF); // 0xFF表示无效索引
        std::fill_n(weights, 3, 0);

        const int maxWeights = std::min(4, (int)src.weights.size());
        validWeights = maxWeights;

        for (int i = 0; i < maxWeights; ++i) {
            jointIndices[i] = static_cast<uint8_t>(src.weights[i].jointIndex);
            if (i < 3) { 
                weights[i] = static_cast<uint8_t>(src.weights[i].weight * 255.0f);
            }
        }
    }
};


class Triangle {
public:
    int v0, v1, v2; // Indices of vertices in the Skin's vertex array

    Triangle(int vertex0, int vertex1, int vertex2)
        : v0(vertex0), v1(vertex1), v2(vertex2) {}


    glm::vec3 computeNormal(const std::vector<SkinVertex>& vertices) const {
        glm::vec3 p0 = vertices[v0].position;
        glm::vec3 p1 = vertices[v1].position;
        glm::vec3 p2 = vertices[v2].position;
        return glm::normalize(glm::cross(p1 - p0, p2 - p0));
    }

    glm::vec3 computeCentroid(const std::vector<Vertex>& vertices) const {
        return (vertices[v0].position + vertices[v1].position + vertices[v2].position) / 3.0f;
    }
};

