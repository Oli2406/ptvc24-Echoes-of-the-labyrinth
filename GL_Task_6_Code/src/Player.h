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
	PxRigidDynamic* physxModel;
	PxVec3 position;
	float rotX, rotY, rotZ;
	float scale;
	const float RUN_SPEED = 20.0f;
	const float TURN_SPEED = 160.0f;
	float speed = 0;
	float turnSpeed = 0;
	const double PI = 3.14159265358979323846;
	const float GRAVITY = -9.81f;
	const float JUMP_POWER = 5.0f;
	float upwardSpeed = 0;
	const float TERRAIN_HEIGHT = 0;
	boolean isInAir = false;
	boolean jumping = false;
	float prevCameraDirectionX = 0.0f;
	float moveForce = 2.0f;
	glm::mat4 modelMatrix;

public:

	Player(Model model, float rotX, float rotY, float rotZ, float scale, PxRigidDynamic* physxModel)
	{
		this->model = model;
		this->position = physxModel->getGlobalPose().p;
		this->rotX = rotX;
		this->rotY = rotY;
		this->rotZ = rotZ;
		this->scale = scale;
		this->physxModel = physxModel;
		PxTransform initialTransform(PxVec3(2.0f, 1.0f, 2.0f));
		this->physxModel->setGlobalPose(initialTransform);
	}


	float getScale() {
		return scale;
	}

	float getRotX() {
		return rotX;
	}

	float getRotY() {
		return rotY;
	}

	float getRotZ() {
		return rotZ;
	}

	glm::vec3 getPosition() {
		PxVec3 physxPos = physxModel->getGlobalPose().p;
		return glm::vec3(physxPos.x, physxPos.y, physxPos.z);
	}

	glm::vec3 getPos() {
		return getPosition();
	}

	Model getModel() {
		return model;
	}

	void updateModelMatrix() {
		PxTransform transform = physxModel->getGlobalPose();
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
		this->position = physxModel->getGlobalPose().p;
		this->rotX = rotX;
		this->rotY = rotY;
		this->rotZ = rotZ;
		this->scale = scale;
	}

	void jump(float delta) {
		if (!isInAir && jumping) {
			upwardSpeed = JUMP_POWER;
			isInAir = true;
		}
		if (isInAir) {
			PxVec3 velocity = physxModel->getLinearVelocity();
			velocity.y = upwardSpeed;
			physxModel->setLinearVelocity(velocity);
			upwardSpeed += GRAVITY * delta;
		}
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

		PxVec3 velocity = physxModel->getLinearVelocity();

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			velocity += PxVec3(horizontalDirection.x, 0.0, horizontalDirection.z) * moveForce * delta;
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			velocity -= PxVec3(horizontalDirection.x, 0.0, horizontalDirection.z) * moveForce * delta;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			velocity -= PxVec3(verticalDirection.x, 0.0, verticalDirection.z) * moveForce * delta;
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			velocity += PxVec3(verticalDirection.x, 0.0, verticalDirection.z) * moveForce * delta;
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !jumping && !isInAir) {
			velocity.y = JUMP_POWER;
			jumping = true;
			isInAir = true;
		}

		physxModel->setLinearVelocity(velocity);
	}


};