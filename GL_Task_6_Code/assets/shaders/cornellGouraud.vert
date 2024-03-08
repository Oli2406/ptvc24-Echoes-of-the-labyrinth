#version 430 core
/*
* Copyright 2023 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the GCG Lab Framework and must not be redistributed.
*/

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 3) in vec3 color;
out VertexData {
	vec4 color;
} vert;

uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;
uniform mat3 normalMatrix;

uniform vec3 camera_world;

uniform vec3 materialCoefficients; // x = ambient, y = diffuse, z = specular 
uniform float specularAlpha;

uniform bool draw_normals;

uniform struct DirectionalLight {
	vec3 color;
	vec3 direction;
} dirL;

uniform struct PointLight {
	vec3 color;
	vec3 position;
	vec3 attenuation;
} pointL;

vec3 phong(vec3 n, vec3 l, vec3 v, vec3 diffuseC, float diffuseF, vec3 specularC, float specularF, float alpha, bool attenuate, vec3 attenuation) {
	float d = length(l);
	l = normalize(l);
	float att = 1.0;	
	if(attenuate) att = 1.0f / (attenuation.x + d * attenuation.y + d * d * attenuation.z);
	vec3 r = reflect(-l, n);
	return (diffuseF * diffuseC * max(0, dot(n, l)) + specularF * specularC * pow(max(0, dot(r, v)), alpha)) * att; 
}

void main() {
	vec3 normal_world = normalMatrix * normal;
	vec4 position_world = modelMatrix * vec4(position, 1);
	gl_Position = viewProjMatrix * position_world;

	vec3 n = normalize(normal_world);
	vec3 v = normalize(camera_world - position_world.xyz);
	
	// Use a different illumination depending on whether we see the inside or outside of the Cornell Box:
	if (dot(n, v) > 0.0) {
		// Assume that the Cornell Box is emissive inside:
		vert.color = vec4(color, 1.0);
		if (draw_normals) {
			vert.color = vec4(n, 1.0);
		}
		return;
	}
	// else, i.e. when viewed from the outside, use the ordinarily shaded color.
	// But attention: We have to invert the normal, because we'd like to illuminate the back faces:
	n = -n;

	vert.color = vec4(color * materialCoefficients.x, 1); // ambient
	
	// add directional light contribution
	vert.color.rgb += phong(n, -dirL.direction, v, dirL.color * color, materialCoefficients.y, dirL.color, materialCoefficients.z, specularAlpha, false, vec3(0));
			
	// add point light contribution
	vert.color.rgb += phong(n, pointL.position - position_world.xyz, v, pointL.color * color, materialCoefficients.y, pointL.color, materialCoefficients.z, specularAlpha, true, pointL.attenuation);

	if (draw_normals) {
		vert.color = vec4(n, 1.0);
	}
}