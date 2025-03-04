#include "Rope.h"
#include <glm/glm.hpp>

void Rope::initialize(const glm::vec3& start, const glm::vec3& end, int numParticles) {
    particles.clear();
    springs.clear();

    // 计算粒子之间的间隔
    glm::vec3 step = (end - start) / float(numParticles - 1);
    for (int i = 0; i < numParticles; ++i) {
        glm::vec3 pos = start + step * float(i);
        // 可根据需要固定首端或双端（此处仅固定第一个粒子）
        bool fixed = (i == 0);
        particles.emplace_back(pos, 1.0f, fixed);
    }

    // 为相邻粒子创建弹簧阻尼器
    for (int i = 0; i < numParticles - 1; ++i) {
        springs.emplace_back(&particles[i], &particles[i + 1], springConstant, dampingFactor);
    }
}

void Rope::update(float dt, const glm::vec3& gravity) {
    // 对所有粒子施加重力
    for (auto& p : particles) {
        p.applyForce(gravity * p.mass);
    }

    // 计算并应用每个弹簧的力
    for (auto& spring : springs) {
        spring.applyForce();
    }

    // 更新粒子状态，并处理简单的地面碰撞（假设地面 y=0）
    for (auto& p : particles) {
        p.update(dt);
        if (p.position.y < 0.0f) {
            p.position.y = 0.0f;
            p.velocity.y = 0.0f;
        }
    }
}
