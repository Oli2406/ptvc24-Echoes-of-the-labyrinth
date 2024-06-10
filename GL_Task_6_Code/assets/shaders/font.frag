#version 330 core
in vec2 TexCoords;
out vec4 color;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    float brightness = dot(textColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(textColor, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

	 color = vec4(textColor, 1.0) * sampled;
}