#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 u_OrthoMatrix;

void main() {
    gl_Position = u_OrthoMatrix * vec4(aPos, 1.0);
}
