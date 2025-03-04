// ClothTriangle.h
#pragma once

#include "Triangle.h"   // Inherited base class
#include "Particle.h"   // For particle positions
#include <glm/glm.hpp>

// Derived class for cloth simulation triangles.
// It extends the original Triangle by storing pointers to the actual Particle objects.
class ClothTriangle : public Triangle {
public:
    Particle* p1;
    Particle* p2;
    Particle* p3;
    glm::vec3 normal;  // Dynamically computed per update

    // We call the base constructor with dummy indices (or you can choose to store indices as well)
    ClothTriangle(Particle* p1, Particle* p2, Particle* p3)
        : Triangle(0, 0, 0), p1(p1), p2(p2), p3(p3) {}

    // Compute the normal using the current positions of the particles.
    void computeNormal() {
        glm::vec3 edge1 = p2->position - p1->position;
        glm::vec3 edge2 = p3->position - p1->position;
        assert(p1->position != p2->position && p3->position != p1->position);
        normal = glm::normalize(glm::cross(edge1, edge2));
    }
};
