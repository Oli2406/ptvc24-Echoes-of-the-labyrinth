#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace glm;

class ArcCamera {
private:
	float myFov; float n; float f; float aspectRatio; float radius;
	mat4 viewMatrix;
	float yaw;
	float pitch;
	float sensitivity;
	vec3 pos;


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

	mat4 calculateMatrix(float radius, float pitch, float yaw) {
		//compute camera Position with Euler Angles
		float x = radius * sin(yaw) * cos(pitch);
		float y = radius * sin(pitch);
		float z = radius * cos(yaw) * cos(pitch);
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
		radius = glm::clamp(radius, 1.0f, 10.0f);
	}
};