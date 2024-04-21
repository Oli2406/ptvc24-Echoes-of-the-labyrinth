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

using namespace std;

class Player
{
private:
	Model model;
	glm::vec3 position;
	glm::vec3 pos = glm::vec3(0, 0, 0);
	float rotX, rotY, rotZ;
	float scale;
	const float RUN_SPEED = 20.0f;
	const float TURN_SPEED = 160.0f;
	float speed = 0;
	float turnSpeed = 0;
	const double PI = 3.14159265358979323846; // Manuelle Definition von PI
	const float GRAVITY = -9.81f;
	const float JUMP_POWER = 5.0f;
	float upwardSpeed = 0;
	const float TERRAIN_HEIGHT = 0;
	boolean isInAir = false;
	boolean jumping = false;
	float prevCameraDirectionX = 0.0f;

public:
	Player(){
		position = glm::vec3(0, 0, 0);
		rotX = 0.5f;
		rotY = 0.5f;
		rotZ = 0.5f;
		scale = 0;
	}

	Player(Model model, glm::vec3 position, float rotX, float rotY, float rotZ, float scale) 
	{
		this->model = model;
		this->position = position;
		this->rotX = rotX;
		this->rotY = rotY;
		this->rotZ = rotZ;
		this->scale = scale;
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
		glm::vec3 now = position - pos;
		pos = position;
		return now;
	}

	glm::vec3 getPos() {
		return position;
	}

	Model getModel() {
		return model;
	}

	void Draw(std::shared_ptr<Shader> shader) {
		this->model.Draw(shader);
	}

	void set(Model model, glm::vec3 position, float rotX, float rotY, float rotZ, float scale) {
		this->model = model;
		this->position = position;
		this->rotX = rotX;
		this->rotY = rotY;
		this->rotZ = rotZ;
		this->scale = scale;
	}

	void increasePosition(float dx, float dy, float dz) {
		position.x += dx;
		position.y += dy;
		position.z += dz;
	}

	void increaseRotation(float dx, float dy, float dz) {
		rotX += dx;
		rotY += dy;
		rotZ += dz;
	}

	void jump(float delta) {
		if (!isInAir && jumping) {
			upwardSpeed = JUMP_POWER;
			isInAir = true;
		}
		if (isInAir) {
			position.y += upwardSpeed * delta;
			upwardSpeed += GRAVITY * delta;

			if (position.y <= TERRAIN_HEIGHT) {
				position.y = TERRAIN_HEIGHT;
				upwardSpeed = 0.0f;
				isInAir = false;
			}
		}
	}

	void updateRotation(glm::vec3 cameraDirection) {

		float yaw = atan2(-cameraDirection.y, -cameraDirection.x);

		float degreesYaw = glm::degrees(yaw);

		if (degreesYaw < -180.0f) {
			degreesYaw += 360.0f;
		}
		else if (degreesYaw > 180.0f) {
			degreesYaw -= 360.0f;
		}

		rotY = degreesYaw;
	}

	glm::vec3 getRotation() {
		return glm::vec3(rotX, rotY, rotZ);
	}


	void checkInputs(GLFWwindow* window, float delta, glm::vec3 direction) {
		glm::vec3 horizontalDirection = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			position += horizontalDirection * delta;
			
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			position -= horizontalDirection * delta;
			
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			position -= glm::normalize(glm::cross(horizontalDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * delta;
			
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			position += glm::normalize(glm::cross(horizontalDirection, glm::vec3(0.0f, 1.0f, 0.0f))) * delta;
			
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			jumping = true;
			jump(delta);
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			jumping = false;
			jump(delta);
		}
	}
};