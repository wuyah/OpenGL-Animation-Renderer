#pragma once

#include "IKController.h"

class IKControlPoint;

class IKSolver {
public:

	IKSolver() : joints(0) {
		
	};

	void initialize(std::vector<std::shared_ptr<Joint>>& jointChain) { joints = jointChain; };

	//std::vector<glm::vec3> computeJacobian(const glm::vec3& endEffectorPos, const glm::vec3& targetPos);

	void SolveIK_Transpose(
		const IKControlPoint& target,
		int maxIterations,
		float threshold,
		float alpha  
	);
	void SolveIK_Transpose_3DOF(
		const glm::vec3& targetPos,
		int maxIterations,
		float threshold,
		float alpha
	);
private:
	std::vector<std::shared_ptr<Joint>> joints;
};