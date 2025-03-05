#pragma once

#include "SkeletonManager.h"

class IKSystem {
public:
	Skeleton* targetPose;
	std::vector<Joint*> targetsJoints;

	Skeleton* sourcePose;
	std::vector<Joint*> sourceJoints;

	IKSystem();
	~IKSystem();

private:

};

IKSystem::IKSystem() {
}

IKSystem::~IKSystem() {
}
