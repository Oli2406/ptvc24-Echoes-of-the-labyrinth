#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

void main()
{
    // Abtastung der Schattentextur
    float depthValue = texture(depthMap, TexCoords).r;

    // Debug-Ausgabe
    // Debug-Ausgabe für Tiefenwert
    if (depthValue == 1.0)
        FragColor = vec4(1.0); // Weiß für sichtbare Bereiche der Schattentextur
    else
        FragColor = vec4(0.0); // Schwarz für verdeckte Bereiche der Schattentextur
}