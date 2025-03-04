// ClothManager.h
#pragma once
#include "Cloth.h"
#include "ClothRenderer.h"
#include "Camera.h"

class ClothManager {
private:
    Cloth cloth;
    ClothRenderer renderer;
    Camera* camera;

    double lastTime;

public:
    int numWidth = 20, numHeight = 20;
    float spacing = 0.05f, stiffness = 3000, damper = 10;
    glm::vec3 origin = glm::vec3(-2, 2, 3);
    
    float groundLevel = -10.f;

    // Initialization functions to set up a cloth simulation.
    bool initializeCloth();

    // Bind a camera (for rendering).
    void bindCamera(Camera* cam) { camera = cam; }

    // Update simulation and renderer.
    void Update(float dt);

    // Render cloth.
    void render(const glm::mat4& viewProjMatrix, GLuint shaderProgram);

    // Expose functions to adjust simulation parameters (e.g., wind, fixed points)
    void setWind(const glm::vec3& wind) { cloth.setWind(wind); }
    void moveFixedParticles(const glm::vec3& delta) { cloth.moveFixedParticles(delta); }
    ClothRenderer* getRenderer() { return &renderer; }
    Cloth* getCloth() { return &cloth; }
};
