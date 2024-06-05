#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in ivec4 boneIds; 
layout(location = 4) in vec4 weights;
	
uniform mat4 viewProjMatrix;
uniform mat4 modelMatrix;
	
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
	
out vec2 TexCoords;
	
void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        int boneID = boneIds[i];
        float weight = weights[i];
        mat4 boneMatrix = finalBonesMatrices[boneID];
        totalPosition += boneMatrix * vec4(pos, 1.0) * weight;
    }
    vec4 finalPosition = modelMatrix * totalPosition;
    gl_Position = viewProjMatrix * finalPosition;
    TexCoords = tex;
}