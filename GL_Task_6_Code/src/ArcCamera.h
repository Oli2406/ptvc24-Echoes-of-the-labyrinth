#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Player.h"

using namespace glm;

class ArcCamera {
private:
	float myFov; float n; float f; float aspectRatio; float radius;
	mat4 viewMatrix;
	float yaw;
	float pitch;
	float sensitivity;
	vec3 pos;
	const float PI = 3.14f;

public:
	ArcCamera() {
		//default values.
		radius = 1.0f;
		sensitivity = 0.005f;
	}


	void setCamParameters(float fov, float aspect, float nearly, float farly, float yawly, float pitchly) {
		//constructor for read paramaters from file.
		myFov = radians(fov);
		aspectRatio = aspect;
		n = nearly;
		f = farly;
		yaw = yawly;
		pitch = pitchly;
	}

	mat4 calculateMatrix(float radius, float pitch, float yaw, Player& player) {
		//compute camera Position with Euler Angles
		float x = radius * sin(yaw) * cos(pitch) - player.getPosition().x;
		float y = radius * sin(pitch) + player.getPosition().y;
		float z = radius * cos(yaw) * cos(pitch) + player.getPosition().z;
		vec3 position(-x, y, z);
		pos = position;
		mat4 viewMatrix = translate(mat4(1.0f), position);
		viewMatrix = glm::rotate(viewMatrix, yaw, vec3(0.0f, -1.0f, 0.0f));
		viewMatrix = glm::rotate(viewMatrix, pitch, vec3(-1.0f, 0.0f, 0.0f));
		viewMatrix = inverse(viewMatrix);
		return viewMatrix;
	}

	vec3 extractCameraDirection(mat4 viewMatrix) {
		mat4 invViewMatrix = inverse(viewMatrix);
		vec3 cameraDirection = -vec3(invViewMatrix[2]);
		return normalize(cameraDirection);
	}


	float getRadius() {
		return radius;
	}

	float getPitch() {
		return pitch;
	}

	float getYaw() {
		return yaw;
	}

	vec3 getPos() {
		return pos;
	}

	void rotate(float xoffset, float yoffset, float sensitivity = 0.005f) {
		pitch += xoffset * sensitivity;
		yaw += yoffset * sensitivity;
		pitch = glm::clamp(pitch, radians(-89.0f), radians(89.0f));
	}


	void zoom(float yoffset) {
		//Calculate the zoom and limit it.
		radius -= yoffset;
		radius = glm::clamp(radius, 2.0f, 2.0f);
	}
};