#include "AnimationClip.h"
#include "Tokenizer.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

bool AnimationClip::Load(const char* filename) {
    Tokenizer tokenizer;
    if (!tokenizer.Open(filename)) {
        fprintf(stderr, "AnimationClip::Load - Unable to open file: %s\n", filename);
        return false;
    }

    char token[256];

    // Expect the "animation" keyword
    if (!tokenizer.GetToken(token) || strcmp(token, "animation") != 0) {
        fprintf(stderr, "AnimationClip::Load - Expected 'animation', got '%s'\n", token);
        return false;
    }

    // Expect opening brace for the animation block
    if (!tokenizer.GetToken(token) || strcmp(token, "{") != 0) {
        fprintf(stderr, "AnimationClip::Load - Expected '{' after 'animation', got '%s'\n", token);
        return false;
    }

    // Read the animation range.
    if (!tokenizer.GetToken(token) || strcmp(token, "range") != 0) {
        fprintf(stderr, "AnimationClip::Load - Expected 'range', got '%s'\n", token);
        return false;
    }
    rangeStart = tokenizer.GetFloat();
    rangeEnd = tokenizer.GetFloat();

    // Read the number of channels.
    if (!tokenizer.GetToken(token) || strcmp(token, "numchannels") != 0) {
        fprintf(stderr, "AnimationClip::Load - Expected 'numchannels', got '%s'\n", token);
        return false;
    }
    int numChannels = tokenizer.GetInt();

    // Process each channel.
    for (int i = 0; i < numChannels; i++) {
        // Read the "channel" keyword.
        if (!tokenizer.GetToken(token) || strcmp(token, "channel") != 0) {
            fprintf(stderr, "AnimationClip::Load - Expected 'channel', got '%s'\n", token);
            return false;
        }
        // Read the channel index token.
        if (!tokenizer.GetToken(token)) {
            fprintf(stderr, "AnimationClip::Load - Expected channel index token.\n");
            return false;
        }
        // Check if the token includes the opening brace (e.g., "14{").
        int len = (int)strlen(token);
        if (token[len - 1] == '{') {
            token[len - 1] = '\0'; // Remove the '{'
            // Optionally, you could convert the remaining string to an integer here.
        }
        else {
            // Otherwise, expect a separate '{' token.
            if (!tokenizer.GetToken(token) || strcmp(token, "{") != 0) {
                fprintf(stderr, "AnimationClip::Load - Expected '{' after channel index, got '%s'\n", token);
                return false;
            }
        }

        // Load the channel data.
        Channel channel;
        if (!channel.Load(tokenizer)) {
            fprintf(stderr, "AnimationClip::Load - Failed to load channel %d\n", i);
            return false;
        }
        channels.push_back(channel);

        // Expect the closing brace for the channel block.
        if (!tokenizer.GetToken(token) || strcmp(token, "}") != 0) {
            fprintf(stderr, "AnimationClip::Load - Expected '}' after channel block, got '%s'\n", token);
            return false;
        }
    }

    // Expect the closing brace for the animation block.
    if (!tokenizer.GetToken(token) || strcmp(token, "}") != 0) {
        fprintf(stderr, "AnimationClip::Load - Expected closing '}' for animation block, got '%s'\n", token);
        return false;
    }

    tokenizer.Close();
    return true;
}


void AnimationClip::Evaluate(float time, std::vector<std::shared_ptr<Joint>>& joints) {
    // Assertion
    assert(!joints.empty() && "AnimationClip::Evaluate: Joint list is empty.");
    const size_t channelsPerJoint = 3;
    time *= 2;
    // Assert that we have enough channels to animate every joint.
    assert(channels.size() == (joints.size() + 1) * channelsPerJoint &&
        "AnimationClip::Evaluate: Insufficient channels for joints.");

    // First one translation
    float rx = channels[0].Evaluate(time);
    float ry = channels[1].Evaluate(time);
    float rz = channels[2].Evaluate(time);
    joints[0]->offset = joints[0]->originOffset + glm::vec3(rx, ry, rz);


    // For each joint, evaluate the corresponding channels and update its pose.
    for (size_t j = 0; j < joints.size(); j++) {
        size_t baseChannel = (j + 1)* channelsPerJoint;

        // Evaluate each channel at the given time.
        float rx = channels[baseChannel].Evaluate(time);
        float ry = channels[baseChannel + 1].Evaluate(time);
        float rz = channels[baseChannel + 2].Evaluate(time);

        // Update the joint's pose. (Here, 'pose' holds Euler angles.)
        joints[j]->pose = glm::vec3(rx, ry, rz);
    }
}
