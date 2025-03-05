#pragma once

#include "core.h"
#include "glm/gtx/quaternion.hpp"
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <stdexcept>

class Joint {
public:
    std::string name;
    glm::vec3 offset;
    glm::vec3 originOffset;
    glm::vec3 boxMin;
    glm::vec3 boxMax;
    glm::vec3 pose; // Pose as Euler angles for easy updates
    glm::vec3 orginalPos;
    glm::vec2 rotXLimit;
    glm::vec2 rotYLimit;
    glm::vec2 rotZLimit;
    std::vector<std::shared_ptr<Joint>> children;
    glm::mat4 localMatrix;
    glm::mat4 worldMatrix;

    Joint* parent;

    Joint(const std::string& name = "");

    void addChild(const std::shared_ptr<Joint>& child);
    void computeLocalMatrix();
    void update(const glm::mat4& parentTransform);
};

class Skeleton {
private:
    std::shared_ptr<Joint> root;
    glm::vec3 position;
    glm::quat rotation;
    glm::mat4 worldMatrix;
    std::vector<std::shared_ptr<Joint>> jointList; // 新增的成员，用于存储所有Joint

    void traverseJointsRecursive(const std::shared_ptr<Joint>& joint, 
                    const std::function<void(const std::shared_ptr<Joint>&)>& callback) const {
        callback(joint);
        for (const auto& child : joint->children) {
            traverseJointsRecursive(child, callback);
        }
    }
    void buildJointListRecursive(const std::shared_ptr<Joint>& joint);

public:
    Skeleton();
    Skeleton(const std::shared_ptr<Joint>& rootJoint, const glm::vec3 pos = glm::vec3(0.f), const glm::quat rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    void buildJointList();


    const std::vector <std::shared_ptr<Joint>> getJointData() const;

    void setRoot(const std::shared_ptr<Joint>& rootJoint);
    const std::shared_ptr<Joint>& getRoot() const;

    void setPosition(const glm::vec3& pos);
    const glm::vec3& getPosition() const;

    void setRotation(const glm::quat& rot);
    const glm::quat& getRotation() const;

    void update();

    glm::mat4 getJointWorldMatrix(size_t index) const {
        if (index >= jointList.size()) {
            throw std::out_of_range("Joint index out of range");
        }
        return jointList[index]->worldMatrix;
    }

    std::vector<std::shared_ptr<Joint>>& getJointList() {
        return jointList;
    }

    //void traverseJoints(const std::function<void(const std::shared_ptr<Joint>&)>& callback) const {
    //    if (root) {
    //        callback(root);
    //    }
    //}


};
