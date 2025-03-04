// Particle.h
#pragma once
#include <glm/glm.hpp>

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 forceAccum;
    float mass;
    bool fixed; // true if this particle is fixed in space

    Particle(const glm::vec3& pos, float mass, bool fixed = false)
        : position(pos), velocity(0.0f), forceAccum(0.0f), mass(mass), fixed(fixed) {}

    // Apply an external force
    void applyForce(const glm::vec3& force) {
        forceAccum += force;
    }
    void applyImpulse(glm::vec3 impulse) {
        if (!fixed) {
            velocity += impulse / mass;
        }
    }

    // Update the particle's state with a simple explicit integration
    void update(float dt) {
        if (fixed) return;  // Do nothing if fixed
        glm::vec3 acceleration = forceAccum / mass;
        velocity += acceleration * dt;
        position += velocity * dt;
        forceAccum = glm::vec3(0.0f); // Reset force accumulator
    }
};
