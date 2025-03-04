#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Particle.h"
#include "SpringDamper.h"

class Rope {
public:
    std::vector<Particle> particles;
    std::vector<SpringDamper> springs;

    // 参数：弹簧常数和阻尼系数
    float springConstant;
    float dampingFactor;

    Rope() : springConstant(100.0f), dampingFactor(0.5f) {}

    // 从 start 到 end 创建一根由 numParticles 个粒子构成的绳子
    void initialize(const glm::vec3& start, const glm::vec3& end, int numParticles);

    // 更新绳子的物理状态，dt为时间步长，gravity为重力
    void update(float dt, const glm::vec3& gravity);
};
