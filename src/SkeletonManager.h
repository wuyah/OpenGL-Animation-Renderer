#pragma once

#include "IKController.h"
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
    std::shared_ptr<Skin> skin; // Use a smart pointer

    std::unique_ptr<AnimationClip> clip;

    IKController ik;

    Camera* camera;

    float currentAnimTime;

    double lastTime;
 
public:
    bool playAnim = false;
    bool enableSkeletonEdit = true;

    bool initializeSkeleton(const std::string& skeletonFileName);
    bool initializeSkin(const std::string& skinFileName);
    bool initializeAnim(const std::string& animFileName);

    void initilizeIK();

    bool initialize(const std::string& skeletonFileName, const std::string& skinFileName, const std::string& animFileName);
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

    IKController* getIK() {
        return &this->ik;
    }

    void cleanUp();

    bool haveclip() {
        return clip ? true : false;
    }
};
