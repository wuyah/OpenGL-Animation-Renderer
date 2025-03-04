#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "Cloth.h"
#include "Material.h"
#include "Lights.h"

class ClothRenderer {
private:
    GLuint VAO, VBO, EBO;
    GLuint indexCount;  // Number of indices.

    // Interleaved vertex data: [pos.x, pos.y, pos.z, norm.x, norm.y, norm.z] for each particle.
    std::vector<GLfloat> vertexData;
    std::vector<GLuint> indexData;

    GLuint groundVAO, groundVBO, groundEBO;
    bool groundInitialized = false;
    float groundLevel = 0.0f;  // 地面的 y 坐标
    float groundSize = 100.0f; // 地面平面的半边长

    // New: material and directional light for proper lighting.
    Material material, groundMaterial;
    DirectionalLight directLight = DirectionalLight(glm::vec3{ 0, 0, 1 }, glm::vec3(1.0), 1.f);
    PointLight pointLight = PointLight(glm::vec3{ 3, 3, 3 }, glm::vec3(0, 0, 1), 100.f);

    // Setup GPU buffers from cloth data.
    void setupBuffers(const Cloth& cloth);
    // Recompute per-vertex normals based on cloth triangles.
    void computeNormals(const Cloth& cloth, std::vector<glm::vec3>& normals);

public:
    ClothRenderer();
    ~ClothRenderer();

    // Initialize buffers based on the provided cloth.
    void initialize(const Cloth& cloth);
    // Update GPU buffers with the current cloth simulation state.
    void update(const Cloth& cloth);
    // Render the cloth.
    void render(const glm::mat4& viewProjMatrix, GLuint shaderProgram);
    // Cleanup GPU resources.
    void cleanup();
    PointLight* getPointLight() { return &pointLight; }
    DirectionalLight* getDirectLight() { return &directLight; };
    Material* getMaterial() { return &material; };

    // Ground Level renderer
    void initializeGround(float level, float size = 10.0f);
    void updateGroundGeometry(float newLevel);
    void renderGround(const glm::mat4& viewProjMatrix, GLuint shaderProgram);


};
