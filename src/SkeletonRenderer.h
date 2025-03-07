// SkeletonRenderer.h
#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Cube.h"
#include "Skeleton.h"
#include "Skin.h"
#include "Material.h"
#include "Lights.h"

enum class SkeletonRenderMode {
    Fill,
    Wireframe
};

class SkeletonRenderer {
private:
    GLuint VAO, VBO, EBO, VBO_normals;
    std::vector<GLfloat> vertexData;
    std::vector<GLuint> indexData;
    std::vector<GLfloat> normalData;
    size_t totalBones;

    Skeleton* skeleton;
    SkeletonRenderMode renderMode = SkeletonRenderMode::Fill;
    bool render_skin = true;
    std::shared_ptr<Skin> skin;
    void setupCubeBuffers();
    void setupSkinBuffersCPU();
    void setupSkinBuffersGPU();
    Material material;
    DirectionalLight directLight = DirectionalLight( glm::vec3{0, 1, 0}, glm::vec3(1.0), 1.f);
    PointLight pointLight = PointLight(glm::vec3{ 3, 3, 0 }, glm::vec3(1.0), 1.f);


public:
    SkeletonRenderer();
    ~SkeletonRenderer();
    void update();
    void cleanup();

    void initialize(Skeleton& skel, std::shared_ptr<Skin> skin);
    void initialize(Skeleton& skel);
    void initializeMaterial() {
        material = Material( glm::vec3(0.2f),
             glm::vec3(0.8f),
             glm::vec3(1.0f), 3.0f);
    }
    //void initialize(Skeleton& skel, SkeletonRenderMode render_mode);

    // Main render function
    void render(const glm::mat4& viewProjMatrix, GLuint shaderProgram, const glm::vec3 cameraPos);
    void renderSkinCPU(const glm::mat4& viewProjMatrix, GLuint shaderProgram, const glm::vec3 cameraPos);
    void renderSkinGPU(const glm::mat4& viewProjMatrix, GLuint shaderProgram, const glm::vec3 cameraPos);

    void setRenderMode(SkeletonRenderMode mode) { renderMode = mode; }

    SkeletonRenderMode getRenderMode() const { return renderMode; }

    void updateSkinVerticesCPU();

    Material* getMaterial() { return &material; }
    DirectionalLight* getDirectLight() { return &directLight; }
    PointLight* getPointLight() { return &pointLight; }

    glm::vec3 renderColor = glm::vec3(1.f);
    bool renderInGPU = false;

};