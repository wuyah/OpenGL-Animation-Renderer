// SpringDamper.h
#pragma once
#include "Particle.h"

class SpringDamper {
public:
    Particle* p1;
    Particle* p2;
    float restLength;
    float stiffness;  // Spring constant 
    float damping;    // Damping coefficient k_d

    SpringDamper(Particle* p1, Particle* p2, float stiffness, float damping)
        : p1(p1), p2(p2), stiffness(stiffness), damping(damping)
    {
        restLength = glm::length(p1->position - p2->position);
    }

    // Compute and apply the spring-damper force to both particles
    void applyForce() {
        glm::vec3 delta = p1->position - p2->position;
        float currentLength = glm::length(delta);
        if (currentLength == 0.0f) return;
        glm::vec3 direction = delta / currentLength;

        // Hooke's law: F = -k(x - x0)
        float springForce = -stiffness * (currentLength - restLength);

        // Damping force: F = -c(v_rel dot direction)
        glm::vec3 relativeVel = p1->velocity - p2->velocity;
        float dampingForce = -damping * glm::dot(relativeVel, direction);

        glm::vec3 force = (springForce + dampingForce) * direction;

        p1->applyForce(force);
        p2->applyForce(-force);
    }
};
