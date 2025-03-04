#pragma once

#include "core.h"
#include <vector>
#include <string>
#include <cctype>

// A keyframe holds time, value, and computed tangents,
// plus the tangent mode (as read from the file) for each side.
struct Key {
    float time;
    float value;
    float inTangent;
    float outTangent;
    std::string inTangentMode;   // e.g., "smooth", "linear", "flat"
    std::string outTangentMode;  // e.g., "smooth", "linear", "flat"
};

class Channel {
public:
    std::string extrapolateIn;
    std::string extrapolateOut;
    std::vector<Key> keys;

    // Returns the interpolated value at the given time.
    float Evaluate(float time);

    // Loads channel data from the file using our Tokenizer.
    bool Load(class Tokenizer& tokenizer);

    // Precompute the in/out tangents for each key based on the tangent modes.
    void precomputeTangents();
};
