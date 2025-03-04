#pragma once

#include "core.h"
#include <vector>
#include "Skeleton.h"
#include "Channel.h"  // our channel definition below

class AnimationClip {
public:
    // The overall time range for the animation.
    float rangeStart;
    float rangeEnd;
    // One channel per animated DOF (for example)
    std::vector<Channel> channels;

    void Evaluate(float time, std::vector<std::shared_ptr<Joint>>& joints);
    bool Load(const char* filename);
};
