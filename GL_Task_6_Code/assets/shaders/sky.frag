#version 430 core
out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 texCoords;

uniform samplerCube skybox;

void main()
{    
    vec3 texColor = texture(skybox, texCoords).rgb;
    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(texColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(texColor, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    FragColor = vec4(texColor, 1.0);
}