#include "ClothRenderer.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

// Helper: Compute per-vertex normals by averaging the normals of all triangles incident to each particle.
void ClothRenderer::computeNormals(const Cloth& cloth, std::vector<glm::vec3>& normals) {
    // Initialize normals to zero.
    normals.resize(cloth.particles.size(), glm::vec3(0.0f));

    // For each cloth triangle (ClothTriangle) accumulate its normal to each vertex.
    for (const auto& tri : cloth.triangles) {
        // Compute triangle normal using the three particles.
        glm::vec3 edge1 = tri.p2->position - tri.p1->position;
        glm::vec3 edge2 = tri.p3->position - tri.p1->position;
        glm::vec3 triNormal = glm::normalize(glm::cross(edge1, edge2));

        // Find indices of the particles in cloth.particles.
        auto idx1 = std::find_if(cloth.particles.begin(), cloth.particles.end(), [&](const Particle& p) { return &p == tri.p1; }) - cloth.particles.begin();
        auto idx2 = std::find_if(cloth.particles.begin(), cloth.particles.end(), [&](const Particle& p) { return &p == tri.p2; }) - cloth.particles.begin();
        auto idx3 = std::find_if(cloth.particles.begin(), cloth.particles.end(), [&](const Particle& p) { return &p == tri.p3; }) - cloth.particles.begin();

        // Accumulate the triangle normal to each vertex.
        if (idx1 < normals.size()) normals[idx1] += triNormal;
        if (idx2 < normals.size()) normals[idx2] += triNormal;
        if (idx3 < normals.size()) normals[idx3] += triNormal;
    }

    // Normalize all normals.
    for (auto& n : normals) {
        n = glm::normalize(n);
    }
}

void ClothRenderer::setupBuffers(const Cloth& cloth) {
    // Compute per-vertex normals.
    std::vector<glm::vec3> normals;
    computeNormals(cloth, normals);

    // Build interleaved vertexData.
    vertexData.clear();
    for (size_t i = 0; i < cloth.particles.size(); ++i) {
        const auto& pos = cloth.particles[i].position;
        vertexData.push_back(pos.x);
        vertexData.push_back(pos.y);
        vertexData.push_back(pos.z);

        // Use computed normal; if for some reason the cloth has no triangles, default to (0,1,0).
        if (i < normals.size()) {
            vertexData.push_back(normals[i].x);
            vertexData.push_back(normals[i].y);
            vertexData.push_back(normals[i].z);
        }
        else {
            vertexData.push_back(0.0f);
            vertexData.push_back(1.0f);
            vertexData.push_back(0.0f);
        }
    }

    // Build index data from cloth triangles.
    indexData.clear();
    for (const auto& tri : cloth.triangles) {
        GLuint i1 = static_cast<GLuint>(
            std::find_if(cloth.particles.begin(), cloth.particles.end(),
                [&](const Particle& par) { return &par == tri.p1; }) - cloth.particles.begin());
        GLuint i2 = static_cast<GLuint>(
            std::find_if(cloth.particles.begin(), cloth.particles.end(),
                [&](const Particle& par) { return &par == tri.p2; }) - cloth.particles.begin());
        GLuint i3 = static_cast<GLuint>(
            std::find_if(cloth.particles.begin(), cloth.particles.end(),
                [&](const Particle& par) { return &par == tri.p3; }) - cloth.particles.begin());
        indexData.push_back(i1);
        indexData.push_back(i2);
        indexData.push_back(i3);
    }
    indexCount = static_cast<GLuint>(indexData.size());

    // Generate and bind VAO.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create and populate VBO with interleaved vertex data.
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);

    // Attribute 0: position (3 floats), attribute 1: normal (3 floats) interleaved.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Create and populate EBO with index data.
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void ClothRenderer::updateGroundGeometry(float newLevel) {
    groundLevel = newLevel;
    float halfSize = groundSize;
    std::vector<GLfloat> groundVertices = {
        -halfSize, groundLevel, -halfSize,
         halfSize, groundLevel, -halfSize,
         halfSize, groundLevel,  halfSize,
        -halfSize, groundLevel,  halfSize
    };

    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, groundVertices.size() * sizeof(GLfloat), groundVertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Set up Ground Level Geometry
void ClothRenderer::initializeGround(float level, float size) {
    groundLevel = level;
    groundSize = size;
    float halfSize = groundSize;

    // 构造地面顶点：构造一个平面，位于 y = groundLevel
    // 顶点顺序（按逆时针排列）：
    // (-halfSize, groundLevel, -halfSize)
    // ( halfSize, groundLevel, -halfSize)
    // ( halfSize, groundLevel,  halfSize)
    // (-halfSize, groundLevel,  halfSize)
    std::vector<GLfloat> groundVertices = {
        -halfSize, groundLevel, -halfSize,
         halfSize, groundLevel, -halfSize,
         halfSize, groundLevel,  halfSize,
        -halfSize, groundLevel,  halfSize
    };

    // 两个三角形构成一个矩形
    std::vector<GLuint> groundIndices = { 0, 1, 2, 0, 2, 3 };

    glGenVertexArrays(1, &groundVAO);
    glBindVertexArray(groundVAO);

    glGenBuffers(1, &groundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, groundVertices.size() * sizeof(GLfloat), groundVertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &groundEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, groundIndices.size() * sizeof(GLuint), groundIndices.data(), GL_STATIC_DRAW);

    // 设置顶点属性：假定位置属性在 layout(location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    groundInitialized = true;
}

// render Ground
void ClothRenderer::renderGround(const glm::mat4& viewProjMatrix, GLuint shaderProgram) {
    if (!groundInitialized) return;

    glUseProgram(shaderProgram);
    GLuint skinModeLoc = glGetUniformLocation(shaderProgram, "useGPUSkinning");
    glUniform1i(skinModeLoc, 0);

    groundMaterial.SetUniforms(shaderProgram, "material");
    directLight.SetUniforms(shaderProgram, "dirLight");
    pointLight.SetUniforms(shaderProgram, "pointLight");

    // 设置 viewProj 矩阵 uniform
    GLint vpLoc = glGetUniformLocation(shaderProgram, "viewProj");
    glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(viewProjMatrix));

    // 设置 model 矩阵（此处使用单位矩阵，若需要可以加上平移/缩放）
    glm::mat4 model = glm::mat4(1.0f);
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


    glBindVertexArray(groundVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}


ClothRenderer::ClothRenderer()
    : VAO(0), VBO(0), EBO(0), indexCount(0),
    // Initialize with a material similar to SkeletonRenderer.
    material(glm::vec3(0.2f), glm::vec3(0.8f), glm::vec3(1.0f), 32.0f) {
}

ClothRenderer::~ClothRenderer() {
    cleanup();
}

void ClothRenderer::initialize(const Cloth& cloth) {
    setupBuffers(cloth);
}

void ClothRenderer::update(const Cloth& cloth) {
    // Recompute normals from the updated cloth state.
    std::vector<glm::vec3> normals;
    computeNormals(cloth, normals);

    // Update interleaved vertex data.
    size_t numParticles = cloth.particles.size();
    if (vertexData.size() != numParticles * 6) {
        vertexData.resize(numParticles * 6);
    }
    for (size_t i = 0; i < numParticles; ++i) {
        const auto& pos = cloth.particles[i].position;
        vertexData[i * 6 + 0] = pos.x;
        vertexData[i * 6 + 1] = pos.y;
        vertexData[i * 6 + 2] = pos.z;
        if (i < normals.size()) {
            vertexData[i * 6 + 3] = normals[i].x;
            vertexData[i * 6 + 4] = normals[i].y;
            vertexData[i * 6 + 5] = normals[i].z;
        }
        else {
            vertexData[i * 6 + 3] = 0.0f;
            vertexData[i * 6 + 4] = 1.0f;
            vertexData[i * 6 + 5] = 0.0f;
        }
    }

    // Update VBO with new vertex data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(GLfloat), vertexData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ClothRenderer::render(const glm::mat4& viewProjMatrix, GLuint shaderProgram) {
    glUseProgram(shaderProgram);

    GLuint skinModeLoc = glGetUniformLocation(shaderProgram, "useGPUSkinning");
    glUniform1i(skinModeLoc, 0);


    // Set shader uniforms (similar to SkeletonRenderer).
    GLint vpLoc = glGetUniformLocation(shaderProgram, "viewProj");
    glUniformMatrix4fv(vpLoc, 1, GL_FALSE, glm::value_ptr(viewProjMatrix));

    // Optional: set a model matrix (here identity)
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Set material uniforms.
    material.SetUniforms(shaderProgram, "material");
    directLight.SetUniforms(shaderProgram, "dirLight");
    pointLight.SetUniforms(shaderProgram, "pointLight");

    // Draw cloth.
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void ClothRenderer::cleanup() {
    if (EBO) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    if (VBO) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
}
