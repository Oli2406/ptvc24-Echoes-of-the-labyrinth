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
	struct FrustumPlane {
		glm::vec3 normal;
		float distance;

		FrustumPlane() : normal(0.0f), distance(0.0f) {}
		FrustumPlane(const glm::vec3& normal, float distance) : normal(normal), distance(distance) {}

		float distanceToPoint(const glm::vec3& point) const {
			return glm::dot(normal, point) + distance;
		}
	};

	FrustumPlane frustumPlanes[6];

public:

	ArcCamera() {
		//default values.
		radius = 3.0f;
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
		float y = radius * sin(pitch) + player.getPosition().y + 0.25f;
		float z = radius * cos(yaw) * cos(pitch) + player.getPosition().z;
		vec3 position(-x, y, z);
		pos = position;
		mat4 viewMatrix = translate(mat4(1.0f), position);
		viewMatrix = glm::rotate(viewMatrix, yaw, vec3(0.0f, -1.0f, 0.0f));
		viewMatrix = glm::rotate(viewMatrix, pitch, vec3(-1.0f, 0.0f, 0.0f));
		viewMatrix = inverse(viewMatrix);
		return viewMatrix;
	}

	mat4 calculateMatrix2(float radius, float pitch, float yaw, float forward, float left) {
		//compute camera Position with Euler Angles
		float x = radius * sin(yaw) * cos(pitch) + left;
		float y = radius * sin(pitch);
		float z = radius * cos(yaw) * cos(pitch) + forward;
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
		radius -= yoffset;
		radius = glm::clamp(radius, 3.0f, 3.0f);
	}
	void ArcCamera::updateFrustumPlanes(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
		glm::mat4 clip = projectionMatrix * viewMatrix;

		frustumPlanes[0].normal = glm::vec3(clip[0][3] - clip[0][0],
			clip[1][3] - clip[1][0],
			clip[2][3] - clip[2][0]);
		frustumPlanes[0].distance = clip[3][3] - clip[3][0];


		frustumPlanes[1].normal = glm::vec3(clip[0][3] + clip[0][0],
			clip[1][3] + clip[1][0],
			clip[2][3] + clip[2][0]);
		frustumPlanes[1].distance = clip[3][3] + clip[3][0];

		frustumPlanes[2].normal = glm::vec3(clip[0][3] + clip[0][1],
			clip[1][3] + clip[1][1],
			clip[2][3] + clip[2][1]);
		frustumPlanes[2].distance = clip[3][3] + clip[3][1];

		frustumPlanes[3].normal = glm::vec3(clip[0][3] - clip[0][1],
			clip[1][3] - clip[1][1],
			clip[2][3] - clip[2][1]);
		frustumPlanes[3].distance = clip[3][3] - clip[3][1];

		frustumPlanes[4].normal = glm::vec3(clip[0][3] - clip[0][2],
			clip[1][3] - clip[1][2],
			clip[2][3] - clip[2][2]);
		frustumPlanes[4].distance = clip[3][3] - clip[3][2];

		frustumPlanes[5].normal = glm::vec3(clip[0][3] + clip[0][2],
			clip[1][3] + clip[1][2],
			clip[2][3] + clip[2][2]);
		frustumPlanes[5].distance = clip[3][3] + clip[3][2];

		for (int i = 0; i < 6; ++i) {
			float length = glm::length(frustumPlanes[i].normal);
			frustumPlanes[i].normal /= length;
			frustumPlanes[i].distance /= length;
		}
	}

	bool ArcCamera::isPointInFrustum(const glm::vec3& point) const {
		for (int i = 0; i < 6; ++i) {
			if (frustumPlanes[i].distanceToPoint(point) < 0) {
				return false;
			}
		}
		return true;
	}

	bool ArcCamera::isSphereInFrustum(const glm::vec3& center, float radius) const {
		for (int i = 0; i < 6; ++i) {
			if (frustumPlanes[i].distanceToPoint(center) < -radius) {
				return false;
			}
		}
		return true;
	}
};
