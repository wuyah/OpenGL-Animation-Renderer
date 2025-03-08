// Skeleton.cpp
#include "Skeleton.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <queue>
#include <algorithm>

// Joint class implementation
Joint::Joint(const std::string& name)
    : name(name),
    offset(0.0f),
    boxMin(0.0f),
    boxMax(0.0f),
    pose(0.0f),
    rotXLimit(0.0f),
    rotYLimit(0.0f),
    rotZLimit(0.0f),
    localMatrix(1.0f),
    worldMatrix(1.0f), 
    children(0) {}

void Joint::addChild(const std::shared_ptr<Joint>& child) {
    children.push_back(child);
}

void Joint::computeLocalMatrix() {
    glm::mat4 offsetMat = glm::translate(glm::mat4(1.0f), offset);
    
    pose.x = std::clamp(pose.x, rotXLimit.x, rotXLimit.y);
    pose.y = std::clamp(pose.y, rotYLimit.x, rotYLimit.y);
    pose.z = std::clamp(pose.z, rotZLimit.x, rotZLimit.y);


    glm::quat orientation = glm::quat(pose);
    glm::mat4 rotationMat = glm::mat4_cast(orientation);
    
    localMatrix = offsetMat * rotationMat;
}

void Joint::update(const glm::mat4& parentTransform) {
    computeLocalMatrix();
    worldMatrix = parentTransform * localMatrix;
    for (const auto& child : children) {
        child->update(worldMatrix);
    }
}

glm::vec3 Joint::getWorldPosition() {
    return glm::vec3(worldMatrix[3]);
}

glm::vec3 Joint::getEndPointWorldPos() {
    return glm::vec3(worldMatrix[3]) + offset;
}

void Joint::updateWorldMatrix() {
    if (parent) {
        // Compute worldMatrix by combining parent's worldMatrix with localMatrix
        worldMatrix = parent->worldMatrix * localMatrix;
    }
    else {
        // Root joint: worldMatrix is just the localMatrix
        worldMatrix = localMatrix;
    }
    if (children.size() > 0) {
        // Recursively update children
        for (const auto& child : children) {
            child->updateWorldMatrix();
        }
    }
}


void Joint::setPosition(const glm::vec3& newWorldPos) {
    if (parent) {
        // Compute local position relative to parent
        glm::mat4 parentInv = glm::inverse(parent->worldMatrix);
        glm::vec4 localPos = parentInv * glm::vec4(newWorldPos, 1.0f);
        offset = glm::vec3(localPos); // Update offset
    }
    else {
        // Root joint - world position is directly set
        offset = newWorldPos;
    }

    // Update transformations
    computeLocalMatrix();
    updateWorldMatrix();
}

// Skeleton class implementation
Skeleton::Skeleton()
    : worldMatrix(glm::identity<glm::mat4>()) {}

Skeleton::Skeleton(const std::shared_ptr<Joint>& rootJoint, const glm::vec3 pos, const glm::quat rot)
    : root(rootJoint),
    position(pos),
    rotation(rot),
    worldMatrix(glm::translate(glm::mat4(1.0f), position)* glm::mat4_cast(rotation)) {
}

const std::vector<std::shared_ptr<Joint>> Skeleton::getJointData() const {
    //std::vector<Joint> jointData;
    //std::queue<const Joint*> jointQueue;

    //jointQueue.push(root.get());

    //while (!jointQueue.empty()) {
    //    const Joint* joint = jointQueue.front();
    //    jointQueue.pop();
    //    jointData.push_back(*joint); 

    //    for (const auto& child : joint->children) {
    //        jointQueue.push(child.get());
    //    }
    //}

    //return jointData;
    return jointList;
}



void Skeleton::setRoot(const std::shared_ptr<Joint>& rootJoint) {
    root = rootJoint;
}

const std::shared_ptr<Joint>& Skeleton::getRoot() const {
    return root;
}

void Skeleton::setPosition(const glm::vec3& pos) {
    position = pos;
    worldMatrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);
}

const glm::vec3& Skeleton::getPosition() const {
    return position;
}

void Skeleton::setRotation(const glm::quat& rot) {
    rotation = rot;
    worldMatrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);
}

const glm::quat& Skeleton::getRotation() const {
    return rotation;
}

void Skeleton::update() {
    if (root) {
        worldMatrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation);
        root->update(worldMatrix);
    }
}

void Skeleton::buildJointList() {
    jointList.clear();
    buildJointListRecursive(root);
}

void Skeleton::buildJointListRecursive(const std::shared_ptr<Joint>& joint) {
    if (!joint) return;

    jointList.push_back(joint); // Add current joint first (Pre-order)

    for (const auto& child : joint->children) {
        buildJointListRecursive(child); // Process children
    }
}
