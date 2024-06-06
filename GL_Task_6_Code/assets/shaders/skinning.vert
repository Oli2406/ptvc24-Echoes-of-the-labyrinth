#version 430 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in ivec4 boneIds; 
layout(location = 4) in vec4 weights;

uniform mat4 viewProjMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec3 out_normals;
out vec2 TexCoords;
out vec3 position_world;
out vec4 FragPosLightSpace;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        int boneID = boneIds[i];
        float weight = weights[i];
        mat4 boneMatrix = finalBonesMatrices[boneID];
        totalPosition += boneMatrix * vec4(pos, 1.0) * weight;
        totalNormal += (boneMatrix * vec4(norm, 0.0)).xyz * weight;
    }
    vec4 finalPosition = modelMatrix * totalPosition;
    gl_Position = viewProjMatrix * finalPosition;
    out_normals = normalize(normalMatrix * totalNormal);
    TexCoords = tex;
    position_world = finalPosition.xyz;
    FragPosLightSpace = lightSpaceMatrix * vec4(position_world, 1.0);
}
