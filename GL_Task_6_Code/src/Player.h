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
    const float JUMP_POWER = 5.0f;
    bool isInAir = false;
    const float GRAVITY = -9.81f;
    float moveForce = 1.f;
    glm::mat4 modelMatrix;
    PxVec3 velocity;

public:
    Player(Model model, float rotX, float rotY, float rotZ, float scale, PxController* characterController)
        : model(model), characterController(characterController), velocity(0.0f, 0.0f, 0.0f)
    {
        this->position = PxVec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z);
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

        glm::vec3 glmPosition(playerPos.x, playerPos.y - 1.57296f, playerPos.z);
        position = playerPos;
        glm::quat glmOrientation(orientation.w, orientation.x, orientation.y, orientation.z);

        modelMatrix = glm::translate(glm::mat4(1.0f), glmPosition);
    }

    void Draw(std::shared_ptr<Shader> shader) {
        this->updateModelMatrix();
        shader->setUniform("modelMatrix", modelMatrix);
        this->model.Draw(shader);
    }

    void set(Model model) {
        this->model = model;
        this->position = PxVec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z);
        
    }

    void updatePlayerRotation(PxController* controller, glm::vec3 camDir) {
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 right = glm::normalize(glm::cross(up, camDir));
        glm::vec3 forward = glm::normalize(glm::cross(right, up));

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix[0] = glm::vec4(right, 0.0f);
        rotationMatrix[2] = glm::vec4(forward, 0.0f);

        glm::quat rotationQuaternion = glm::quat_cast(rotationMatrix);
        PxQuat pxRotationQuaternion(rotationQuaternion.x, rotationQuaternion.y, rotationQuaternion.z, rotationQuaternion.w);

        setPlayerRotation(controller, pxRotationQuaternion);
    }

    void setPlayerRotation(PxController* controller, const PxQuat& rotation) {
        PxTransform currentTransform = controller->getActor()->getGlobalPose();
        currentTransform.q = rotation;
        controller->getActor()->setGlobalPose(currentTransform);
    }

    void checkInputs(GLFWwindow* window, float delta, glm::vec3 direction) {
        glm::vec3 horizontalDirection = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
        glm::vec3 verticalDirection = glm::normalize(glm::cross(horizontalDirection, glm::vec3(0.0f, 1.0f, 0.0f)));

        PxVec3 displacement(0.0f, 0.0f, 0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            displacement += (PxVec3(horizontalDirection.x, 0.0f, horizontalDirection.z) * delta) * 4;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            displacement += (PxVec3(-horizontalDirection.x, 0.0f, -horizontalDirection.z) * delta) * 4;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            displacement += (PxVec3(-verticalDirection.x, 0.0f, -verticalDirection.z) * delta) * 4;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            displacement += (PxVec3(verticalDirection.x, 0.0f, verticalDirection.z) * delta) * 4;
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
