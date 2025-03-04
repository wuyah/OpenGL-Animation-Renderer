#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Particle.h"
#include "SpringDamper.h"

class Rope {
public:
    std::vector<Particle> particles;
    std::vector<SpringDamper> springs;

    // ���������ɳ���������ϵ��
    float springConstant;
    float dampingFactor;

    Rope() : springConstant(100.0f), dampingFactor(0.5f) {}

    // �� start �� end ����һ���� numParticles �����ӹ��ɵ�����
    void initialize(const glm::vec3& start, const glm::vec3& end, int numParticles);

    // �������ӵ�����״̬��dtΪʱ�䲽����gravityΪ����
    void update(float dt, const glm::vec3& gravity);
};
