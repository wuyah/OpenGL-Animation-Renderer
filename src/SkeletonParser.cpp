#include "SkeletonParser.h"

bool SkeletonParser::parseJoint(const std::shared_ptr<Joint>& parent, const glm::mat4& parentTransform) {
    char token[256];

    if (!tokenizer.GetToken(token)) return false;
    std::string jointName(token);

    if (!tokenizer.GetToken(token) || strcmp(token, "{") != 0) {
        char buffer[100];
        strcpy(buffer, "Expected '{' after joint name");
        tokenizer.Abort(buffer);
        return false;
    }

    auto joint = std::make_shared<Joint>(jointName);
    if (parent) {
        parent->addChild(joint);
    }
    else {
        skeleton.setRoot(joint);
    }

    joint->offset = glm::vec3(0.0f);
    joint->pose = glm::vec3(0.0f);
    joint->rotXLimit = glm::vec2(-glm::pi<float>(), glm::pi<float>());
    joint->rotYLimit = glm::vec2(-glm::pi<float>(), glm::pi<float>());
    joint->rotZLimit = glm::vec2(-glm::pi<float>(), glm::pi<float>());

    while (true) {
        if (!tokenizer.GetToken(token)) return false;

        if (strcmp(token, "}") == 0) {
            break;
        }
        else if (strcmp(token, "offset") == 0) {
            joint->offset.x = tokenizer.GetFloat();
            joint->offset.y = tokenizer.GetFloat();
            joint->offset.z = tokenizer.GetFloat();
            joint->originOffset.x = joint->offset.x;
            joint->originOffset.y = joint->offset.y;
            joint->originOffset.z = joint->offset.z;
        }
        else if (strcmp(token, "boxmin") == 0) {
            joint->boxMin.x = tokenizer.GetFloat();
            joint->boxMin.y = tokenizer.GetFloat();
            joint->boxMin.z = tokenizer.GetFloat();
        }
        else if (strcmp(token, "boxmax") == 0) {
            joint->boxMax.x = tokenizer.GetFloat();
            joint->boxMax.y = tokenizer.GetFloat();
            joint->boxMax.z = tokenizer.GetFloat();
        }
        else if (strcmp(token, "pose") == 0) {
            joint->pose.x = tokenizer.GetFloat();
            joint->pose.y = tokenizer.GetFloat();
            joint->pose.z = tokenizer.GetFloat();
            joint->orginalPos.x = joint->pose.x;
            joint->orginalPos.y = joint->pose.y;
            joint->orginalPos.z = joint->pose.z;
        }
        else if (strcmp(token, "rotxlimit") == 0) {
            joint->rotXLimit.x = tokenizer.GetFloat();
            joint->rotXLimit.y = tokenizer.GetFloat();
        }
        else if (strcmp(token, "rotylimit") == 0) {
            joint->rotYLimit.x = tokenizer.GetFloat();
            joint->rotYLimit.y = tokenizer.GetFloat();
        }
        else if (strcmp(token, "rotzlimit") == 0) {
            joint->rotZLimit.x = tokenizer.GetFloat();
            joint->rotZLimit.y = tokenizer.GetFloat();
        }
        else if (strcmp(token, "balljoint") == 0) {
            if (!parseJoint(joint, joint->worldMatrix)) {
                return false;
            }
        }
        else {
            tokenizer.SkipLine();
        }
    }

    joint->computeLocalMatrix();
    joint->worldMatrix = parentTransform * joint->localMatrix;

    return true;
}

bool SkeletonParser::parseSkeletonFile(const std::string& filename) {
    if (!tokenizer.Open(filename.c_str())) {
        printf("Error opening .skel file: %s\n", filename.c_str());
        return false;
    }

    char token[256];
    if (!tokenizer.GetToken(token) || strcmp(token, "balljoint") != 0) {
        char buffer[100];
        strcpy(buffer, "Expected 'balljoint' at file start");
        tokenizer.Abort(buffer);

        return false;
    }

    if (!parseJoint(nullptr, glm::mat4(1.0f))) {
        return false;
    }

    tokenizer.Close();
    return true;
}

Skeleton& SkeletonParser::getSkeleton() {
    return skeleton;
}

//bool parseSkinFile(const std::string& filename) {
//    return true;
//}

