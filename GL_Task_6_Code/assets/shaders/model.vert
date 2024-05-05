#version 430 core
layout ( location = 0 ) in vec3 position;
layout ( location = 1 ) in vec3 normal;
layout ( location = 2 ) in vec2 texCoords;

out vec3 out_normals;
out vec2 TexCoords;
out vec3 position_world;

uniform mat4 model;
uniform mat4 viewProjMatrix;
uniform mat3 normalMatrix;

void main( )
{
    out_normals = normalMatrix * normal;
    TexCoords = texCoords;
	vec4 position_w = model * vec4( position, 1.0f );
	position_world = position_w.xyz;
	gl_Position = viewProjMatrix *  position_w;
}