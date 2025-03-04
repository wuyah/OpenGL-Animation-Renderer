#include "Rope.h"
#include <glm/glm.hpp>

void Rope::initialize(const glm::vec3& start, const glm::vec3& end, int numParticles) {
    particles.clear();
    springs.clear();

    // ��������֮��ļ��
    glm::vec3 step = (end - start) / float(numParticles - 1);
    for (int i = 0; i < numParticles; ++i) {
        glm::vec3 pos = start + step * float(i);
        // �ɸ�����Ҫ�̶��׶˻�˫�ˣ��˴����̶���һ�����ӣ�
        bool fixed = (i == 0);
        particles.emplace_back(pos, 1.0f, fixed);
    }

    // Ϊ�������Ӵ�������������
    for (int i = 0; i < numParticles - 1; ++i) {
        springs.emplace_back(&particles[i], &particles[i + 1], springConstant, dampingFactor);
    }
}

void Rope::update(float dt, const glm::vec3& gravity) {
    // ����������ʩ������
    for (auto& p : particles) {
        p.applyForce(gravity * p.mass);
    }

    // ���㲢Ӧ��ÿ�����ɵ���
    for (auto& spring : springs) {
        spring.applyForce();
    }

    // ��������״̬��������򵥵ĵ�����ײ��������� y=0��
    for (auto& p : particles) {
        p.update(dt);
        if (p.position.y < 0.0f) {
            p.position.y = 0.0f;
            p.velocity.y = 0.0f;
        }
    }
}
