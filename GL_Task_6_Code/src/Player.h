#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Model.h"
#include <cmath>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace std;

class Player
{
private:
    Model model;
    PxController* characterController;
    PxVec3 position;
    float rotX, rotY, rotZ;
    float scale;
    const float RUN_SPEED = 20.0f;
    const float TURN_SPEED = 160.0f;
    const float JUMP_POWER = 5.0f;
    bool isInAir = false;
    const float GRAVITY = -9.81f;
    float moveForce = 1.f;
    glm::mat4 modelMatrix;
    PxVec3 velocity;

public:
    Player(Model model, float rotX, float rotY, float rotZ, float scale, PxController* characterController)
        : model(model), characterController(characterController), rotX(rotX), rotY(rotY), rotZ(rotZ), scale(scale), velocity(0.0f, 0.0f, 0.0f)
    {
        this->position = PxVec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = rotation * modelMatrix;
    }

    float getScale() const {
        return scale;
    }

    float getRotX() const {
        return rotX;
    }

    float getRotY() const {
        return rotY;
    }

    float getRotZ() const {
        return rotZ;
    }

    glm::vec3 getPosition() const {
        PxExtendedVec3 physxPos = characterController->getPosition();
        return glm::vec3(static_cast<float>(physxPos.x), static_cast<float>(physxPos.y), static_cast<float>(physxPos.z));
    }

    Model getModel() const {
        return model;
    }

    void updateModelMatrix() {
        PxTransform transform = characterController->getActor()->getGlobalPose();
        PxQuat orientation = transform.q;
        PxVec3 playerPos = transform.p;

        glm::vec3 glmPosition(playerPos.x, playerPos.y, playerPos.z);
        position = playerPos;
        glm::quat glmOrientation(orientation.w, orientation.x, orientation.y, orientation.z);

        modelMatrix = glm::translate(glm::mat4(1.0f), glmPosition) * glm::toMat4(glmOrientation);
    }

    void Draw(std::shared_ptr<Shader> shader) {
        this->updateModelMatrix();
        shader->setUniform("modelMatrix", modelMatrix);
        this->model.Draw(shader);
    }

    void set(Model model, float rotX, float rotY, float rotZ, float scale) {
        this->model = model;
        this->position = PxVec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z);
        this->rotX = rotX;
        this->rotY = rotY;
        this->rotZ = rotZ;
        this->scale = scale;
    }

    void updateRotation(glm::vec3 cameraDirection) {
        cameraDirection = glm::normalize(cameraDirection);
        float yaw = atan2(cameraDirection.y, cameraDirection.x);
        float pitch = asin(-cameraDirection.z);
        glm::quat yawQuat = glm::angleAxis(yaw, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::quat pitchQuat = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat rotationQuat = yawQuat * pitchQuat;
        glm::vec3 eulerAngles = glm::eulerAngles(rotationQuat);
        float finalYaw = eulerAngles.z;
        rotY = finalYaw;
    }

    void checkInputs(GLFWwindow* window, float delta, glm::vec3 direction) {
        glm::vec3 horizontalDirection = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
        glm::vec3 verticalDirection = glm::normalize(glm::cross(horizontalDirection, glm::vec3(0.0f, 1.0f, 0.0f)));

        PxVec3 displacement(0.0f, 0.0f, 0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            displacement += PxVec3(horizontalDirection.x, 0.0f, horizontalDirection.z) * delta;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            displacement += PxVec3(-horizontalDirection.x, 0.0f, -horizontalDirection.z) * delta;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            displacement += PxVec3(-verticalDirection.x, 0.0f, -verticalDirection.z) * delta;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            displacement += PxVec3(verticalDirection.x, 0.0f, verticalDirection.z) * delta;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isInAir) {
            velocity.y = JUMP_POWER;
            isInAir = true;
        }

        if (isInAir) {
            velocity.y += GRAVITY * delta;
        }

        displacement.y += velocity.y * delta;

        PxControllerCollisionFlags collisionFlags = characterController->move(displacement, 0.001f, delta, PxControllerFilters());
        if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN) {
            isInAir = false;
            velocity.y = 0.0f;
        }
        else {
            isInAir = true;
        }
    }
};
