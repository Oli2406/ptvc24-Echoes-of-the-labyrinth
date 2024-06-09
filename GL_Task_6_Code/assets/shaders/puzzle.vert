#version 430 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 projection;

void main()
{
    TexCoord = texCoord;
    gl_Position = projection * vec4(position, 0.0, 1.0);
}
