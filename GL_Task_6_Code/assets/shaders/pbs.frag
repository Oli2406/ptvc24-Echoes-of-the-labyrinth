#version 430 core

in vec3 out_normals;
in vec3 position_world;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

out vec4 color;

uniform vec3 camera_world;

uniform sampler2D texture_diffuse;  // Texture unit 0
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D shadowMap;  // Texture unit 1
uniform samplerCube skybox;   // Texture unit 2

uniform bool draw_texcoords;
uniform bool draw_normals;

struct DirectionalLight {
    vec3 color;
    vec3 direction;
};
uniform DirectionalLight dirL;

struct PointLight {
    vec3 color;
    vec3 position;
    vec3 attenuation;  // attenuation.x = constant, attenuation.y = linear, attenuation.z = quadratic
};
uniform PointLight pointL;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float ShadowCalculation(float dotLightNormal)
{
    vec3 pos = FragPosLightSpace.xyz * 0.5 + 0.5;
    if (pos.z > 1.0) {
        pos.z = 1.0;
    }
    float depth = texture(shadowMap, pos.xy).r;
    float bias = max(0.05 * (1.0 - dotLightNormal), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float depth = texture(shadowMap, pos.xy + vec2(x, y) * texelSize).r;
            shadow += (depth + bias) < pos.z ? 0.3 : 1.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec3 albedo = texture(texture_diffuse, TexCoords).rgb;
    vec3 N = normalize(out_normals);
    vec3 V = normalize(camera_world - position_world);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // Directional Light
    vec3 L = normalize(-dirL.direction);
    vec3 H = normalize(V + L);
    
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    float NdotL = max(dot(N, L), 0.0);
    vec3 radiance = dirL.color * NdotL;
    
    vec3 specular = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
    Lo += (kD * albedo / PI + specular) * radiance;

    // Point Light
    vec3 pointLightDir = normalize(pointL.position - position_world);
    vec3 pointH = normalize(V + pointLightDir);
    float pointDistance = length(pointL.position - position_world);
    float pointAttenuation = 1.0 / (pointL.attenuation.x + pointL.attenuation.y * pointDistance + pointL.attenuation.z * pointDistance * pointDistance);
    vec3 pointRadiance = pointL.color * pointAttenuation;
    
    float pointNDF = DistributionGGX(N, pointH, roughness);
    float pointG = GeometrySmith(N, V, pointLightDir, roughness);
    vec3 pointF = fresnelSchlick(max(dot(pointH, V), 0.0), F0);
    
    vec3 pointkS = pointF;
    vec3 pointkD = vec3(1.0) - pointkS;
    pointkD *= 1.0 - metallic;
    
    float pointNdotL = max(dot(N, pointLightDir), 0.0);
    
    vec3 pointSpecular = (pointNDF * pointG * pointF) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
    Lo += (pointkD * albedo / PI + pointSpecular) * pointRadiance * pointNdotL;

    // Image-based lighting reflection
    vec3 R = reflect(-V, N);
    vec3 envColor = texture(skybox, R).rgb;

    // Combine with ambient occlusion
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 finalColor = ambient + (1.0 - ShadowCalculation(dot(-dirL.direction, N))) * Lo;
    finalColor = mix(finalColor, envColor, 0.005f);

    // Tone mapping and gamma correction
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0 / 2.2));
    
    if (draw_normals) {
        finalColor = N;
    }
    if (draw_texcoords) {
        finalColor = vec3(TexCoords, 0);
    }

    color = vec4(finalColor, 1.0);
}
