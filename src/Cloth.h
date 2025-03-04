// Cloth.h
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Particle.h"
#include "SpringDamper.h"
#include "ClothTriangle.h"

class Cloth {
public:
    std::vector<Particle> particles;
    std::vector<SpringDamper> springs;
    std::vector<ClothTriangle> triangles;

    std::vector<Particle*> fixedParticles;

    glm::vec3 gravity;
    glm::vec3 wind;
    float ambientDrag;
    float groundLevel = -100;  // y coordinate of the ground
    float PHYS_EPISILON = 1e-2;
    float restitution = 0.5;

    float rho = 1.225f;
    float Cd = 1.28f;

    Cloth()
        : gravity(0.0f, -9.81f, 0.0f), wind(0.0f), ambientDrag(0.1f), groundLevel(-100.0) {}

    // Example initialization: a rectangular cloth grid.
    void initializeRectangularCloth(int numWidth, int numHeight, float spacing, const glm::vec3& origin, float stiffness, float damper, float mass);

    // Update the simulation by a time step dt.
    void update(float dt);

    void setWind(const glm::vec3& newWind);

    void setGround(const float& newGround) { this->groundLevel = newGround; };

    // Move fixed particles according to user control.
    void moveFixedParticles(const glm::vec3& delta);

    // Apply aero dynamic Force
    void applyAeroDynamic();

};
