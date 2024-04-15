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
	float rotX, rotY, rotZ;
	float scale;
	const float RUN_SPEED = 20.0f;
	const float TURN_SPEED = 160.0f;
	float speed = 0;
	float turnSpeed = 0;
	const double PI = 3.14159265358979323846; // Manuelle Definition von PI
	const float GRAVITY = -50.0f;
	const float JUMP_POWER = 30.0f;
	float upwardSpeed = 0;
	const float TERRAIN_HEIGHT = 0;
	boolean isInAir = false;
	std::string input;
	boolean jumping = false;

public:
	Player(){}

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
<<<<<<< HEAD
    if (!isInAir && jumping) {
		isInAir = true;
        upwardSpeed = 5.0f;
    }
    
    if (isInAir) {
        position.y += upwardSpeed * delta; // Hier sollte 'position.y' erhöht werden
        upwardSpeed += GRAVITY * delta;
		std::cout << position.y << std::endl;
    }
=======
		if (!isInAir && jumping) {
			upwardSpeed = JUMP_POWER;
			isInAir = true;
		}
		if (isInAir) {
			upwardSpeed += GRAVITY * delta;
			position.y = upwardSpeed * delta;
			std::cout << position.y << std::endl;
>>>>>>> 15c17df1f103bb011eb128dd740c9201d437c852

	if (position.y <= 0.0f && upwardSpeed <= -5.0f) {
		std::cout << upwardSpeed << std::endl;
		position.y = 0.0f;
		upwardSpeed = 0.0f;
		isInAir = false;
	}
}

	void checkInputs(GLFWwindow* window, float delta) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			position.x -= 0.1f * delta;
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			position.x += 0.1f * delta;
		}
		else {
			position.x = 0;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			position.z += 0.1f * delta;
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			position.z -= 0.1f * delta;
		}
		else {
			position.z = 0;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			jumping = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			jumping = false;
		}
	}
};