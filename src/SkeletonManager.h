#pragma once

#include <windows.h>
#include <memory>
#include "Skeleton.h"
#include "SkeletonParser.h"
#include "SkeletonRenderer.h"
#include <iostream>
#include "Skin.h"
#include "Camera.h"
#include "AnimationClip.h"

const std::string resourcePath = "..\\resources\\skeletons\\";

class SkeletonManager {
private:
    SkeletonParser parser;
    Skeleton skeleton;
    SkeletonRenderer renderer;
    std::unique_ptr<Skin> skin; // Use a smart pointer

    std::unique_ptr<AnimationClip> clip;

    Camera* camera;

    float currentAnimTime;

    double lastTime;
 
public:
    bool playAnim = false;


    bool initializeSkeleton(const std::string& skeletonFileName);
    bool initializeSkin(const std::string& skinFileName);
    bool initializeAnim(const std::string& animFileName);
    bool initializeRenderer();

    void storeCurrentSkeleton(const std::string& skelStoreFileName, const std::string& filename);

    void Update();

    void draw(const glm::mat4& viewProjMatrix, GLuint shaderProgram);

    void bindCamera(Camera* cam) {
        camera = cam;
    }

    SkeletonRenderer* getRenderer() {
        return &renderer;
    }

    Skeleton* getSkeleton() {
        return &this->skeleton;
    }

    void cleanUp() {
        renderer.cleanup();
    }

    bool haveclip() {
        return clip ? true : false;
    }
};
