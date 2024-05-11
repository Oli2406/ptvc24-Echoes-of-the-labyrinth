#version 430 core
/*
* Copyright 2023 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the GCG Lab Framework and must not be redistributed.
*/

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out VertexData {
	vec3 position_world;
	vec3 normal_world;
	vec2 uv;
} vert;

out vec4 FragPosLightSpace;

uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

void main() {
	vert.normal_world = normalMatrix * normal;
	vert.uv = uv;
	vec4 position_world_ = modelMatrix * vec4(position, 1);
	vert.position_world = position_world_.xyz;
	FragPosLightSpace = lightSpaceMatrix * vec4(vert.position_world, 1.0);
	gl_Position = viewProjMatrix * position_world_;
}