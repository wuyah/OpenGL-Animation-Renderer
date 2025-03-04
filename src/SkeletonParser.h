#pragma once

#include "Tokenizer.h"
#include "Skeleton.h"

class SkeletonParser {
private:
    Tokenizer tokenizer;
    Skeleton skeleton;

    bool parseJoint(const std::shared_ptr<Joint>& parent, const glm::mat4& parentTransform);

public:
    SkeletonParser() = default;

    bool parseSkeletonFile(const std::string& filename);

    bool parseSkinFile(const std::string& filename);

    Skeleton& getSkeleton();
};
