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
	boolean jumping = false;
	boolean isInAir = false;
	const float GRAVITY = -50.0f;
	float upwardSpeed = 0;

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

	mat4 calculateMatrix(float radius, float pitch, float yaw, Player player) {
		//compute camera Position with Euler Angles
		float x = radius * sin(yaw) * cos(pitch) - player.getPos().x;
		float y = radius * sin(pitch) + player.getPos().y;
		float z = radius * cos(yaw) * cos(pitch) + player.getPos().z;
		vec3 position(-x, y, z);
		pos = position;
		mat4 viewMatrix = translate(mat4(1.0f), position);
		viewMatrix = glm::rotate(viewMatrix, yaw, vec3(0.0f, -1.0f, 0.0f));
		viewMatrix = glm::rotate(viewMatrix, pitch, vec3(-1.0f, 0.0f, 0.0f)); //glm because ArcCamera::rotate must not be called here
		viewMatrix = inverse(viewMatrix);
		//translate, rotate and inverse to compute new viewMatrix.
		return viewMatrix;
	}

	//getters used for modifying viewMatrix in Renderloop.
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
		//uses x and y offset to calculate new pitch and yaw.
		pitch += xoffset * sensitivity;
		yaw += yoffset * sensitivity;
		//clamp to nearly 90 degrees to avoid camera inversion.
		pitch = glm::clamp(pitch, radians(-89.0f), radians(89.0f));
	}


	void zoom(float yoffset) {
		//Calculate the zoom and limit it.
		radius -= yoffset;
		radius = glm::clamp(radius, 1.0f, 100.0f);
	}

	void checkInputs(GLFWwindow* window, float delta) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			pos.x -= 0.1f * delta;
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			pos.x += 0.1f * delta;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			pos.z += 0.1f * delta;
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			pos.z -= 0.1f * delta;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			jumping = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
			jumping = false;
		}
	}

	void jump(float delta) {
		if (!isInAir && jumping) {
			upwardSpeed = 10;
			isInAir = true;
		}
		if (isInAir) {
			pos.y += upwardSpeed * delta;
			upwardSpeed += GRAVITY * delta;

			if (pos.y <= 0.0f) {
				pos.y = 0.0f;
				upwardSpeed = 0.0f;
				isInAir = false;
			}
		}
	}
};