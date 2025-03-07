#include "IKSolver.h"

static std::vector<glm::vec3> computeJacobian(
    const std::vector<std::shared_ptr<Joint>>& joints,
    const glm::vec3& endEffectorPos
) {
    int n = static_cast<int>(joints.size());
    std::vector<glm::vec3> J(n);

    for (int i = 0; i < n; ++i) {
        glm::vec3 jointPos = joints[i]->getWorldPosition();

        // If each joint rotates about the Y-axis in world space:
        glm::vec3 axis(0.0f, 1.0f, 0.0f);

        // partial derivative wrt joint i's angle about that axis
        // ∂(endEffector)/∂θᵢ = axis × (endEffectorPos - jointPos)
        J[i] = glm::cross(axis, (endEffectorPos - jointPos));
    }
    return J; // This is 3×n stored “column-wise”
}


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


void IKSolver::SolveIK_Transpose(
    const IKControlPoint& target,
    int maxIterations,
    float threshold,
    float alpha
) {
    if (joints.empty()) return;

    for (int iter = 0; iter < maxIterations; ++iter) {
        // Typically the end-effector is the last joint in the chain
        glm::vec3 endPos = joints.back()->getEndPointWorldPos();

        glm::vec3 error = target.position - endPos;

        // Check convergence
        if (glm::length(error) < threshold)
            break;

        // Compute the Jacobian (3×n)
        std::vector<glm::vec3> J = computeJacobian(joints, endPos);

        // Δθ = α * Jᵀ * error
        std::vector<float> deltaTheta(joints.size(), 0.0f);
        for (size_t i = 0; i < joints.size(); ++i) {
            float dotVal = glm::dot(J[i], error);
            deltaTheta[i] = alpha * dotVal;
        }

        // Apply each Δθ around the Y-axis
        for (size_t i = 0; i < joints.size(); ++i) {
            // If storing per-joint Euler angles, increment pose.y:
            joints[i]->pose.y += deltaTheta[i];
            joints[i]->computeLocalMatrix();
        }
    }
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
        //
        // deltaTheta[3*i + 0] = alpha * dot( J[3*i + 0], error )  // x axis
        // deltaTheta[3*i + 1] = alpha * dot( J[3*i + 1], error )  // y axis
        // deltaTheta[3*i + 2] = alpha * dot( J[3*i + 2], error )  // z axis
        //
        for (size_t i = 0; i < 3 * n; ++i) {
            deltaTheta[i] = alpha * glm::dot(J[i], error);
        }

        // 5) apply each Δθ to the joint's Euler angles
        // For joint i, we have:
        //    Δθx = deltaTheta[3*i + 0]
        //    Δθy = deltaTheta[3*i + 1]
        //    Δθz = deltaTheta[3*i + 2]
        for (size_t i = 0; i < n; ++i) {
            joints[i]->pose.x += deltaTheta[3 * i + 0];
            joints[i]->pose.y += deltaTheta[3 * i + 1];
            joints[i]->pose.z += deltaTheta[3 * i + 2];

            // Recompute the local matrix and ultimately the chain’s world transforms.
            joints[i]->computeLocalMatrix();
        }

        // Possibly re-run a forward kinematics pass from the root outward
        // so that each joint’s worldMatrix is up to date before next iteration.
        // E.g. your skeleton might do something like skeleton.updateWorldMatrices();
    }
}
