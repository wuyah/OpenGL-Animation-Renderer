// Cloth.cpp
#include "Cloth.h"
#include <glm/gtx/compatibility.hpp> // for glm::lerp if needed
#include <iostream>

void Cloth::initializeRectangularCloth(int numWidth, int numHeight, float spacing, const glm::vec3& origin, float stiffness, float damper, float mass) {
    // Clear any existing data.
    particles.clear();
    springs.clear();
    triangles.clear();

    particles.reserve(numWidth * numHeight);
    springs.reserve(4 * numWidth * numHeight - 3 * numWidth - 3 * numHeight + 2);
    triangles.reserve(2 * (numWidth - 1) * (numHeight - 1));
    // Create particles.
    for (int j = 0; j < numHeight; ++j) {
        for (int i = 0; i < numWidth; ++i) {
            glm::vec3 pos = origin + glm::vec3(i * spacing, 0.f, -j * spacing);
            // For example, fix the top row.
            bool fixed = ( (j == 0 && i==0) || (j==0 && i == numWidth-1) );
            particles.emplace_back(pos, mass, fixed);
            if (fixed) {
                fixedParticles.push_back(&particles.back()); 
            }
        }
    }

    // Create structural springs (horizontal and vertical)
    for (int j = 0; j < numHeight; ++j) {
        for (int i = 0; i < numWidth; ++i) {
            int index = j * numWidth + i;

            // Horizontal spring.
            if (i < numWidth - 1) {
                int rightIndex = index + 1;
                springs.emplace_back(&particles[index], &particles[rightIndex], stiffness, damper);
            }

            // Vertical spring.
            if (j < numHeight - 1) {
                int belowIndex = index + numWidth;
                springs.emplace_back(&particles[index], &particles[belowIndex], stiffness, damper);
            }

            // Shear springs: add two diagonals per grid cell.
            if (i < numWidth - 1 && j < numHeight - 1) {
                // Diagonal from top-left to bottom-right.
                int indexBR = (j + 1) * numWidth + (i + 1);
                springs.emplace_back(&particles[index], &particles[indexBR], stiffness, damper);

                // Diagonal from top-right to bottom-left.
                int indexTR = j * numWidth + (i + 1);
                int indexBL = (j + 1) * numWidth + i;
                springs.emplace_back(&particles[indexTR], &particles[indexBL], stiffness, damper);
            }
        }
    }

    // Create cloth triangles (two per grid cell).
    for (int j = 0; j < numHeight - 1; ++j) {
        for (int i = 0; i < numWidth - 1; ++i) {
            int index = j * numWidth + i;
            int indexRight = index + 1;
            int indexBelow = index + numWidth;
            int indexBelowRight = indexBelow + 1;
            // First triangle.
            triangles.emplace_back(&particles[index], &particles[indexBelow], &particles[indexBelowRight]);
            // Second triangle.
            triangles.emplace_back(&particles[index], &particles[indexBelowRight], &particles[indexRight]);
        }
    }
}

void Cloth::update(float dt) {

    applyAeroDynamic();

    // Compute and apply spring-damper forces.
    for (auto& spring : springs) {
        spring.applyForce();
    }
    // Apply forces to particles.
    for (auto& p : particles) {
        // Gravity.
        p.applyForce(gravity * p.mass);

        if (p.position.y <= groundLevel && p.velocity.y != 0) {
            float v_n = p.velocity.y;

            // Compute impulse magnitude:
            // J = - (1 + restitution) * m * v_n
            float impulseMagnitude = -(1.0f + restitution) * v_n * p.mass;
            glm::vec3 impulse(0.0f, impulseMagnitude, 0.f);

            // Apply the impulse to change the velocity.
            p.applyImpulse(impulse);
            p.position.y = groundLevel + PHYS_EPISILON;
        }

        //Wind force (for simplicity, applying uniformly).
       //p.applyForce(wind);
        p.update(dt);

    }

    //// Update particle positions and handle ground collisions.
    //for (auto& p : particles) {
    //}

    // Update triangle normals.
    for (auto& tri : triangles) {
        tri.computeNormal();
    }
}

void Cloth::setWind(const glm::vec3& newWind) {
    wind = newWind;
}

void Cloth::moveFixedParticles(const glm::vec3& delta) {
    for (auto& p : particles) {
        if (p.fixed) {
            p.position += delta;
        }
    }
}

void Cloth::applyAeroDynamic() {

    for (auto& tri : triangles) {
        // Positions & velocities of the 3 vertices
        glm::vec3 p1Pos = tri.p1->position;
        glm::vec3 p2Pos = tri.p2->position;
        glm::vec3 p3Pos = tri.p3->position;

        glm::vec3 v1 = tri.p1->velocity;
        glm::vec3 v2 = tri.p2->velocity;
        glm::vec3 v3 = tri.p3->velocity;

        // Average velocity of the triangle
        glm::vec3 vAvg = (v1 + v2 + v3) / 3.0f;
        glm::vec3 vRel = wind - vAvg; // relative wind velocity

        // Compute edges & normal
        glm::vec3 edge1 = p2Pos - p1Pos;
        glm::vec3 edge2 = p3Pos - p1Pos;
        glm::vec3 normal = glm::cross(edge1, edge2);
        float area = 0.5f * glm::length(normal);
        if (area < 1e-7f) {
            continue; // degenerate triangle
        }
        normal = glm::normalize(normal);

        // Project velocity onto normal
        float vDotN = glm::dot(vRel, normal);
        // The "flat plate" formula: F = -0.5 * rho * |v|^2 * Cd * A * n
        // But we only want the component perpendicular to the surface:
        // sign is determined by dot(vRel, n) (which can be negative)
        float forceMag = 0.5f * rho * (vDotN * vDotN) * Cd * area;
        // If vDotN < 0, the force is in the opposite direction
        // multiply by sign(vDotN):
        float sign = (vDotN >= 0.0f) ? 1.0f : -1.0f;
        glm::vec3 F = -sign * forceMag * normal;

        // Distribute among the three vertices
        glm::vec3 eachVertexForce = F / 3.0f;
        tri.p1->applyForce(eachVertexForce);
        tri.p2->applyForce(eachVertexForce);
        tri.p3->applyForce(eachVertexForce);
    }
}
