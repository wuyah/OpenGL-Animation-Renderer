// ClothManager.cpp
#include "ClothManager.h"
#include <iostream>
#include <GL/glew.h>

bool ClothManager::initializeCloth() {
    // Initialize the cloth simulation.
    cloth.initializeRectangularCloth(numWidth, numHeight, spacing, origin, stiffness, damper, 1);
    cloth.setGround(this->groundLevel);
    cloth.setWind({0.5, 0.5, 0.5});
    // Initialize the renderer with the cloth simulation state.
    renderer.initialize(cloth);
    renderer.initializeGround(groundLevel);
    lastTime = glfwGetTime();
    return true;
}

void ClothManager::Update(float dt) {
    // You might choose a fixed timestep here.
    //float dt = 0.00016f; // ~60 FPS timestep
    cloth.update(dt);
    renderer.update(cloth);
    renderer.updateGroundGeometry(groundLevel);

    cloth.setGround(this->groundLevel);
    // Optionally, update camera-related information if needed.
    if (camera) {
    }
}

void ClothManager::render(const glm::mat4& viewProjMatrix, GLuint shaderProgram) {
    renderer.render(viewProjMatrix, shaderProgram);
    renderer.renderGround(viewProjMatrix, shaderProgram);
}
