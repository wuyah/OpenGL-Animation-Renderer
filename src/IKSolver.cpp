#include "IKSolver.h"

static std::vector<glm::vec3> computeJacobian_3DOF(
    const std::vector<std::shared_ptr<Joint>>& joints,
    const glm::vec3& endEffectorPos
) {
    // We'll store the result in a 1D array where for joint i:
    //   columns = 3 * i + 0, 3 * i + 1, 3 * i + 2
    // Each entry is a glm::vec3 partial derivative.
    //
    // If we have n joints, total columns = 3*n.
    // Each column is d(endEffectorPos)/d(theta_dof).
    size_t n = joints.size();
    std::vector<glm::vec3> J(3 * n);

    for (size_t i = 0; i < n; ++i) {
        // -- 1) get X, Y, Z axes in world space
        // We'll treat joint i's local X/Y/Z as (1,0,0), (0,1,0), (0,0,1),
        // transformed by the joint's current worldMatrix.
        glm::mat4 wMat = joints[i]->worldMatrix;

        glm::vec3 xAxisWorld = glm::normalize(glm::vec3(wMat * glm::vec4(1, 0, 0, 0)));
        glm::vec3 yAxisWorld = glm::normalize(glm::vec3(wMat * glm::vec4(0, 1, 0, 0)));
        glm::vec3 zAxisWorld = glm::normalize(glm::vec3(wMat * glm::vec4(0, 0, 1, 0)));

        // -- 2) joint position
        glm::vec3 jointPos = joints[i]->getWorldPosition();
        glm::vec3 toEff = endEffectorPos - jointPos; // vector from joint i to end effector

        // partial derivative w.r.t. X rotation = xAxisWorld cross (toEff)
        J[3 * i + 0] = glm::cross(xAxisWorld, toEff);
        // partial derivative w.r.t. Y rotation = yAxisWorld cross (toEff)
        J[3 * i + 1] = glm::cross(yAxisWorld, toEff);
        // partial derivative w.r.t. Z rotation = zAxisWorld cross (toEff)
        J[3 * i + 2] = glm::cross(zAxisWorld, toEff);
    }
    return J;
}


void IKSolver::SolveIK_Transpose_3DOF(
    const glm::vec3& targetPos,
    int maxIterations,
    float threshold,
    float alpha
) {
    if (joints.empty()) return;

    for (int iter = 0; iter < maxIterations; ++iter) {
        // 1) get current end-effector position
        glm::vec3 endPos = joints.back()->getWorldPosition();

        // 2) error vector
        glm::vec3 error = targetPos - endPos;
        if (glm::length(error) < threshold) {
            break; // converged
        }

        // 3) compute the Jacobian for 3-DOF
        std::vector<glm::vec3> J = computeJacobian_3DOF(joints, endPos);

        // We'll gather deltas in a 3*n sized array
        size_t n = joints.size();
        std::vector<float> deltaTheta(3 * n, 0.0f);

        // 4) J^T * error => for each column, dot that column with 'error'
        // The column i is J[i], a glm::vec3; dot it with error => a float
        // We'll do 3 columns per joint i:
        for (size_t i = 0; i < 3 * n; ++i) {
            deltaTheta[i] = alpha * glm::dot(J[i], error);
        }

        for (size_t i = 0; i < n; ++i) {
            joints[i]->pose.x += deltaTheta[3 * i + 0];
            joints[i]->pose.y += deltaTheta[3 * i + 1];
            joints[i]->pose.z += deltaTheta[3 * i + 2];

            // Recompute the local matrix and ultimately the chain’s world transforms.
            joints[i]->computeLocalMatrix();
        }
        // we will update the skeleton in ik controller so we dont need to worry the world matrix update here
    }
}

void IKSolver::initializeByPoints(std::vector< IKControlPoint*> inputPoints)
{ 
    controlPoints = inputPoints; 
    makeChains();               // Generate chains, 2D joints
};

void IKSolver::makeChains() {

    chains.clear();

    // A no-op deleter so std::shared_ptr doesn't free the raw pointer
    auto noop_deleter = [](Joint*) {};

    for (auto cp : controlPoints)
    {
        // If cp->boundJoint is a std::shared_ptr<Joint> to your real skeleton joint...
        Joint* cur = cp->boundJoint.get();

        std::vector<std::shared_ptr<Joint>> chain;
        // Root of chain is stored first in the array, so we build from root downward
        // but we find that by walking to the parent, so let's store them in reverse 
        // then reverse() at the end.

        // Add the *actual* pointer (with no-op deleter)
        chain.push_back(std::shared_ptr<Joint>(cur, noop_deleter));

        // Walk up the parent pointers
        while (cur->parent)
        {
            chain.push_back(std::shared_ptr<Joint>(cur->parent, noop_deleter));
            cur = cur->parent;
        }
        // Reverse so that the root is chain[0]
        std::reverse(chain.begin(), chain.end());

        chains.push_back(chain);
    }
}

glm::quat eulerToQuat(const glm::vec3& eulerAngles) {
    return glm::quat(glm::radians(eulerAngles)); // GLM expects radians
}
glm::vec3 quatToEuler(const glm::quat& q) {
    return glm::degrees(glm::eulerAngles(q)); // Convert back to degrees if needed
}
glm::mat4 eulerAnglesToMat4(const glm::vec3& eulerAngles) {
    return glm::toMat4(eulerToQuat(eulerAngles)); // Uses quaternion conversion
}
glm::vec3 orthonormalize(const glm::vec3& v, const glm::vec3& hint = glm::vec3(1, 0, 0)) {
    glm::vec3 ortho = glm::cross(v, hint); // Find a perpendicular vector
    if (glm::length2(ortho) < 1e-6f) {
        ortho = glm::cross(v, glm::vec3(0, 1, 0)); // If parallel, try another axis
    }
    return glm::normalize(ortho);
}

glm::vec3 getForwardDir(Joint* joint) {
    // 1) Build the joint's local rotation from joint->pose (Euler) or a quaternion
    glm::mat4 localRot = eulerAnglesToMat4(joint->pose);
    // 2) The default "forward" can be +Z in local coords
    glm::vec4 localForward(0, 0, 1, 0);
    // 3) Convert to world direction by 
    glm::mat4 parentWorld = joint->parent ?
        joint->parent->worldMatrix :
        glm::mat4(1.0f);
    glm::mat4 worldRot = parentWorld * localRot;
    glm::vec4 dir4 = worldRot * localForward;
    return glm::normalize(glm::vec3(dir4));
}
void rotateJointToFace(Joint* joint, const glm::vec3& jointPos, const glm::vec3& newPos)
{
    glm::vec3 desiredDir = newPos - jointPos;
    float len2 = glm::length2(desiredDir);
    if (len2 < 1e-9f) {
        return; // no rotation if target is basically the same point
    }
    desiredDir = glm::normalize(desiredDir);

    glm::vec3 currentDir = getForwardDir(joint);
    float cosTheta = glm::dot(currentDir, desiredDir);

    if (cosTheta > 0.9999f) {
        // nearly the same direction, no rotation needed
        return;
    }
    // otherwise, build an axis-angle rotation
    glm::vec3 rotAxis = glm::cross(currentDir, desiredDir);
    float axisLen = glm::length(rotAxis);
    if (axisLen < 1e-9f) {
        // directions opposite or nearly so
        // pick an orthonormal axis to rotate 180 deg
        rotAxis = orthonormalize(glm::vec3(1, 0, 0), currentDir);
    }
    else {
        rotAxis = glm::normalize(rotAxis);
    }
    float angle = acosf(glm::clamp(cosTheta, -1.0f, 1.0f));

    // Apply that rotation to the joint’s Euler or quaternion
    // For Euler, you may do something more involved:
    // e.g. convert axis-angle -> quaternion -> euler, or apply minimal rotation on one axis, etc.

    // Pseudocode for quaternion approach:
    glm::quat qRot = glm::angleAxis(angle, rotAxis);
    glm::quat currentQ = eulerToQuat(joint->pose); // your code to convert from Euler to quat
    glm::quat newQ = qRot * currentQ;

    joint->pose = quatToEuler(newQ);    // convert back to Euler if needed
    joint->computeLocalMatrix();
    joint->updateWorldMatrix();
}

void IKSolver::SolveIK_FABRIK() {
    // Typical FABRIK parameters; tune as needed
    const float tolerance = 0.001f;
    const int maxIterations = 10;

    // Solve each chain independently
    for (size_t chainIdx = 0; chainIdx < chains.size(); ++chainIdx)
    {
        auto& chain = chains[chainIdx];
        if (chain.empty()) continue;

        // Assume each IKControlPoint has a 'target' in world space
        glm::vec3 targetPos = controlPoints[chainIdx]->position;

        // 1) Gather current world positions of each Joint in the chain
        std::vector<glm::vec3> jointWorldPositions;
        jointWorldPositions.reserve(chain.size());
        for (auto& jointPtr : chain) {
            jointWorldPositions.push_back(jointPtr->getWorldPosition());
        }

        // Compute distances between consecutive joints (bone lengths)
        std::vector<float> boneLengths;
        boneLengths.reserve(chain.size() - 1);
        float totalLength = 0.0f;
        for (size_t i = 0; i + 1 < chain.size(); ++i) {
            float dist = glm::length(jointWorldPositions[i + 1] - jointWorldPositions[i]);
            boneLengths.push_back(dist);
            totalLength += dist;
        }

        // The root position
        glm::vec3 rootPos = jointWorldPositions[0];
        float distToTarget = glm::length(targetPos - rootPos);

        // 2) If target is unreachable, simply aim each segment at the target (stretch)
        if (distToTarget > totalLength) {
            // "Stretch" pass from root outwards
            for (size_t i = 0; i + 1 < chain.size(); ++i)
            {
                float r = glm::length(targetPos - jointWorldPositions[i]);
                float lambda = boneLengths[i] / r;
                glm::vec3 newPos = glm::mix(jointWorldPositions[i], targetPos, 1.0f - lambda);

                // Rotate Joint i so it points toward newPos
                rotateJointToFace(chain[i].get(),
                    jointWorldPositions[i],
                    newPos);

                // Recompute world position of next joint i+1 from that rotation
                // i.e. joint i+1 = joint i pos + direction * boneLengths[i]
                jointWorldPositions[i + 1] = chain[i]->getWorldPosition() +
                    getForwardDir(chain[i].get()) * boneLengths[i];
            }
        }
        else {
            // 3) If reachable, do iterative FABRIK:
            for (int iteration = 0; iteration < maxIterations; ++iteration)
            {
                // Check if end-effector is already within tolerance
                if (glm::length(jointWorldPositions.back() - targetPos) < tolerance) {
                    break;
                }

                // --- Backward pass (from end-effector to root) ---
                // Move end-effector to target
                jointWorldPositions.back() = targetPos;
                for (int i = (int)chain.size() - 2; i >= 0; --i)
                {
                    // Aim Joint i so that segment i->i+1 has correct length
                    glm::vec3 nextPos = jointWorldPositions[i + 1];
                    float r = glm::length(nextPos - jointWorldPositions[i]);
                    float lambda = boneLengths[i] / r;
                    glm::vec3 newPos = glm::mix(nextPos, jointWorldPositions[i], lambda);

                    // Rotate Joint i so it faces newPos
                    rotateJointToFace(chain[i].get(),
                        jointWorldPositions[i],
                        newPos);

                    // Recompute world pos of joint i from that rotation
                    jointWorldPositions[i] = chain[i]->getWorldPosition();

                    // Then recalc joint i's parent's position, etc.
                    // But in a backward pass, we effectively pull the joint toward child
                    // so we also shift the child’s position:
                    jointWorldPositions[i] = newPos;
                }

                // Re‐store the root position
                jointWorldPositions[0] = rootPos;

                // --- Forward pass (from root to end) ---
                for (size_t i = 0; i + 1 < chain.size(); ++i)
                {
                    float r = glm::length(jointWorldPositions[i + 1] - jointWorldPositions[i]);
                    float lambda = boneLengths[i] / r;
                    glm::vec3 newPos = glm::mix(jointWorldPositions[i], jointWorldPositions[i + 1], 1.0f - lambda);

                    // Rotate Joint i so it points toward newPos
                    rotateJointToFace(chain[i].get(),
                        jointWorldPositions[i],
                        newPos);

                    // Recompute i+1’s position from that rotation
                    jointWorldPositions[i] = chain[i]->getWorldPosition();
                    jointWorldPositions[i + 1] = chain[i]->getWorldPosition() +
                        getForwardDir(chain[i].get()) * boneLengths[i];
                }
            }
        }
    }
    //
    // so that all child joints' world matrices are fully consistent.

}
