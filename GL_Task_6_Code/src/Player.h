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
	const float RUN_SPEED = 10;
	const float TURN_SPEED = 180;
	float speed = 0;
	float turnSpeed = 0;
	const double PI = 3.14159265358979323846; // Manuelle Definition von PI
	const float GRAVITY = -50;
	const float JUMP_POWER = 10;
	float upwardSpeed = 0;
	const float TERRAIN_HEIGHT = 0;
	boolean isInAir = false;
	std::string input;

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

	void jump() {
		if (!isInAir) {
			upwardSpeed = JUMP_POWER;
			isInAir = true;
		}
	}
   
	void move(float delta) {
		checkInputs();
		this->increaseRotation(0, turnSpeed * delta, 0);
		float distance = speed * delta;
		float dx = (float)(distance * sin(rotY * PI / 180.0));
		float dz = (float)(distance * cos(rotY * PI / 180.0));
		this->increasePosition(dx, 0, dz);
		upwardSpeed += GRAVITY * delta;
		this->increasePosition(0, upwardSpeed * delta, 0);
		if (position.y < TERRAIN_HEIGHT) {
			upwardSpeed = 0;
			position.y = TERRAIN_HEIGHT;
			isInAir = false;
		}
	}
	void check(std::string input) {
		this->input = input;
	}

	void checkInputs() {
		if (input == "W") {
			this->speed = RUN_SPEED;
		}
		else if (input == "S") {
			this->speed = -RUN_SPEED;
		}
		else {
			speed = 0;
		}

		if (input == "D") {
			this->turnSpeed = -TURN_SPEED;
		}
		else if (input == "A") {
			this->turnSpeed = TURN_SPEED;
		}
		else {
			turnSpeed = 0;
		}

		if (input == "SPACE") {
			jump();
		}
	}
};