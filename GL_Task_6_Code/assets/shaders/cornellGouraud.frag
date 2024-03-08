#version 430 core
/*
* Copyright 2023 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the GCG Lab Framework and must not be redistributed.
*/

in VertexData {
	vec4 color;
} vert;

out vec4 color;

void main() {	
	color = vert.color;
}

