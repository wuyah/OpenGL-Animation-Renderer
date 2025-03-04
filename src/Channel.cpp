#include "Channel.h"
#include "Tokenizer.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cassert>
#include <cmath>

bool Channel::Load(Tokenizer& tokenizer) {
    char token[256];

    // Parse the "extrapolate" keyword.
    if (!tokenizer.GetToken(token) || strcmp(token, "extrapolate") != 0) {
        fprintf(stderr, "Channel::Load - Expected 'extrapolate', got '%s'\n", token);
        return false;
    }

    // Get the two extrapolation mode tokens.
    if (!tokenizer.GetToken(token)) {
        fprintf(stderr, "Channel::Load - Expected first extrapolation mode token.\n");
        return false;
    }
    extrapolateIn = token;

    if (!tokenizer.GetToken(token)) {
        fprintf(stderr, "Channel::Load - Expected second extrapolation mode token.\n");
        return false;
    }
    extrapolateOut = token;

    // Parse the "keys" keyword.
    if (!tokenizer.GetToken(token) || strcmp(token, "keys") != 0) {
        fprintf(stderr, "Channel::Load - Expected 'keys', got '%s'\n", token);
        return false;
    }

    // Get the number of keys.
    int numKeys = tokenizer.GetInt();

    // Get the opening brace for the key block.
    if (!tokenizer.GetToken(token)) {
        fprintf(stderr, "Channel::Load - Expected '{' for key block.\n");
        return false;
    }
    int len = (int)strlen(token);
    if (token[len - 1] == '{') {
        token[len - 1] = '\0'; // Remove the '{'
    }
    else if (strcmp(token, "{") != 0) {
        fprintf(stderr, "Channel::Load - Expected '{' for key block, got '%s'\n", token);
        return false;
    }

    // Parse each key.
    for (int i = 0; i < numKeys; i++) {
        Key key;
        key.time = tokenizer.GetFloat();
        key.value = tokenizer.GetFloat();

        // Read the tangent mode tokens from the file.
        if (!tokenizer.GetToken(token)) {
            fprintf(stderr, "Channel::Load - Expected token for inTangentMode.\n");
            return false;
        } 
        else {
            if (token[0] == '-' || token[0] == '.' || isdigit(token[0])) {
                // It¡¯s numeric: store the value and mark the mode as "fixed"
                key.inTangent = atof(token);
                key.inTangentMode = "fixed"; // or similar flag
            }
            else {
                key.inTangentMode = token;
                key.inTangent = 0.0f;
            }
        }

        if (!tokenizer.GetToken(token)) {
            fprintf(stderr, "Channel::Load - Expected token for outTangentMode.\n");
            return false;
        }
        else {
            if (token[0] == '-' || token[0] == '.' || isdigit(token[0])) {
                // It¡¯s numeric: store the value and mark the mode as "fixed"
                key.outTangent = atof(token);
                key.outTangentMode = "fixed"; // or similar flag
            }
            else {
                key.outTangentMode = token;
                key.outTangent = 0.0f;
            }

        }

        keys.push_back(key);
    }

    // Expect the closing brace for the keys block.
    if (!tokenizer.GetToken(token) || strcmp(token, "}") != 0) {
        fprintf(stderr, "Channel::Load - Expected '}' after keys, got '%s'\n", token);
        return false;
    }

    // Precompute tangents based on the tangent mode strings.
    precomputeTangents();

    return true;
}

void Channel::precomputeTangents() {
    // If no keys, nothing to do.
    if (keys.empty())
        return;

    // If there's only one key, set its tangents to zero and return.
    if (keys.size() == 1) {
        keys[0].inTangent = 0.0f;
        keys[0].outTangent = 0.0f;
        return;
    }

    // Loop over each key to compute its in and out tangents.
    for (size_t i = 0; i < keys.size(); i++) {
        // --- Compute out tangent for key[i] ---
        if (i == keys.size() - 1) {
            // Last key: use backward difference if mode isn't "flat".
            if (keys[i].outTangentMode == "flat")
                keys[i].outTangent = 0.0f;
            else {
                float dt = keys[i].time - keys[i - 1].time;
                dt = (dt == 0.0f) ? 1.0f : dt;
                keys[i].outTangent = (keys[i].value - keys[i - 1].value) / dt;
            }
        }
        else {
            if (keys[i].outTangentMode == "flat") {
                keys[i].outTangent = 0.0f;
            }
            else if (keys[i].outTangentMode == "linear") {
                float dt = keys[i + 1].time - keys[i].time;
                dt = (dt == 0.0f) ? 1.0f : dt;
                keys[i].outTangent = (keys[i + 1].value - keys[i].value) / dt;
            }
            else if (keys[i].outTangentMode == "smooth") {
                // For the first key, use forward difference.
                if (i == 0) {
                    float dt = keys[i + 1].time - keys[i].time;
                    dt = (dt == 0.0f) ? 1.0f : dt;
                    keys[i].outTangent = (keys[i + 1].value - keys[i].value) / dt;
                }
                else {
                    float dt = keys[i + 1].time - keys[i - 1].time;
                    dt = (dt == 0.0f) ? 1.0f : dt;
                    keys[i].outTangent = (keys[i + 1].value - keys[i - 1].value) / dt;
                }
            }
            else {
                // Default to 0 if the mode is unrecognized.
                keys[i].outTangent = 0.0f;
            }
        }

        // --- Compute in tangent for key[i] ---
        if (i == 0) {
            // First key: use forward difference if mode isn't "flat".
            if (keys[i].inTangentMode == "flat")
                keys[i].inTangent = 0.0f;
            else {
                float dt = keys[i + 1].time - keys[i].time;
                dt = (dt == 0.0f) ? 1.0f : dt;
                keys[i].inTangent = (keys[i + 1].value - keys[i].value) / dt;
            }
        }
        else {
            if (keys[i].inTangentMode == "flat") {
                keys[i].inTangent = 0.0f;
            }
            else if (keys[i].inTangentMode == "linear") {
                float dt = keys[i].time - keys[i - 1].time;
                dt = (dt == 0.0f) ? 1.0f : dt;
                keys[i].inTangent = (keys[i].value - keys[i - 1].value) / dt;
            }
            else if (keys[i].inTangentMode == "smooth") {
                if (i == keys.size() - 1) {
                    // Last key: use backward difference.
                    float dt = keys[i].time - keys[i - 1].time;
                    dt = (dt == 0.0f) ? 1.0f : dt;
                    keys[i].inTangent = (keys[i].value - keys[i - 1].value) / dt;
                }
                else {
                    float dt = keys[i + 1].time - keys[i - 1].time;
                    dt = (dt == 0.0f) ? 1.0f : dt;
                    keys[i].inTangent = (keys[i + 1].value - keys[i - 1].value) / dt;
                }
            }
            else {
                keys[i].inTangent = 0.0f;
            }
        }
    }
}

float Channel::Evaluate(float time) {
    // Return 0 if no keys exist.
    if (keys.empty())
        return 0.0f;

    float tStart = keys.front().time;
    float tEnd = keys.back().time;
    float period = tEnd - tStart;
    float cycleOffset = 0.0f;

    // Handle extrapolation before the first key.
    if (time < tStart) {
        if (extrapolateIn == "constant") {
            return keys[0].value;
        }
        else if (extrapolateIn == "cycle" || extrapolateIn == "cycle_offset") {
            if (period > 0.0f) {
                int cycles = (int)std::floor((time - tStart) / period);
                float tWrapped = std::fmod(time - tStart, period);
                if (tWrapped < 0)
                    tWrapped += period;
                time = tStart + tWrapped;
                if (extrapolateIn == "cycle_offset")
                    cycleOffset = cycles * (keys.back().value - keys.front().value);
            }
        }
    }
    // Handle extrapolation after the last key.
    else if (time > tEnd) {
        if (extrapolateOut == "constant") {
            return keys.back().value;
        }
        else if (extrapolateOut == "cycle" || extrapolateOut == "cycle_offset") {
            if (period > 0.0f) {
                int cycles = (int)std::floor((time - tStart) / period);
                float tWrapped = std::fmod(time - tStart, period);
                if (tWrapped < 0)
                    tWrapped += period;
                // Have optional 
                //time = tStart + tWrapped;
                time = tStart + tWrapped;

                if (extrapolateOut == "cycle_offset")
                    cycleOffset = cycles * (keys.back().value - keys.front().value);
            }
        }
    }

    for (size_t i = 0; i < keys.size() - 1; i++) {
        const Key& k0 = keys[i];
        const Key& k1 = keys[i + 1];
        if (time >= k0.time && time <= k1.time) {
            float dt = k1.time - k0.time;
            if (dt <= 0.0f)
                return k0.value + cycleOffset;

            float s = (time - k0.time) / dt;

            // Cubic Hermite basis functions.
            float h00 = 2 * s * s * s - 3 * s * s + 1;
            float h10 = s * s * s - 2 * s * s + s;
            float h01 = -2 * s * s * s + 3 * s * s;
            float h11 = s * s * s - s * s;

            float value = h00 * k0.value +
                h10 * dt * k0.outTangent +
                h01 * k1.value +
                h11 * dt * k1.inTangent;
            return value + cycleOffset;
        }
    }
    return keys.back().value + cycleOffset;
}
