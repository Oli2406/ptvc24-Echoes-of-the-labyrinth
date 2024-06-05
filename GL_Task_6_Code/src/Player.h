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
    float moveForce = 2.5f;
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

    void updateModelMatrix(glm::vec3 camDir) {
        PxTransform transform = characterController->getActor()->getGlobalPose();
        PxVec3 playerPos = transform.p;

        glm::vec3 forward = glm::normalize(glm::vec3(camDir.x, 0.0f, camDir.z));
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));

        glm::mat4 rotationMatrix(1.0f);
        rotationMatrix[0] = glm::vec4(right, 0.0f);
        rotationMatrix[1] = glm::vec4(up, 0.0f);
        rotationMatrix[2] = glm::vec4(forward, 0.0f);

        glm::vec3 glmPosition(playerPos.x, playerPos.y, playerPos.z);

        modelMatrix = glm::translate(glm::mat4(1.0f), glmPosition) * rotationMatrix;
        modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0.0, 0.0, 1.0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
    }


    void Draw(std::shared_ptr<Shader> shader, glm::vec3 camDir, bool won) {
        if (!won) {
            this->updateModelMatrix(camDir);
        }
        shader->setUniform("modelMatrix", modelMatrix);
        this->model.Draw(shader);
    }

    void set(Model model) {
        this->model = model;
        this->position = PxVec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z);
        
    }

    void updatePlayerRotation(PxController* controller, glm::vec3 camDir) {
        glm::vec3 forward = glm::normalize(camDir);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        up = glm::cross(forward, right);

        glm::mat4 rotationMatrix(1.0f);
        rotationMatrix[0] = glm::vec4(right, 0.0f);
        rotationMatrix[1] = glm::vec4(up, 0.0f);
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
            displacement += (PxVec3(horizontalDirection.x, 0.0f, horizontalDirection.z) * delta) * moveForce;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            displacement += (PxVec3(-horizontalDirection.x, 0.0f, -horizontalDirection.z) * delta) * moveForce;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            displacement += (PxVec3(-verticalDirection.x, 0.0f, -verticalDirection.z) * delta) * moveForce;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            displacement += (PxVec3(verticalDirection.x, 0.0f, verticalDirection.z) * delta) * moveForce;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isInAir) {
            velocity.y = JUMP_POWER;
            isInAir = true;
        }

        if (isInAir) {
            velocity.y += GRAVITY * delta;
        }

        if (characterController->getActor()->getGlobalPose().p.y < -5.0f) {
            characterController->setPosition(PxExtendedVec3(5, 2, 5));
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
