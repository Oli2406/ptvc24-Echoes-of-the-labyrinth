
#version 430 core

in vec3 out_normals;
in vec3 position_world;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 BrightColor;

uniform vec3 camera_world;

uniform sampler2D texture_diffuse;
uniform sampler2D shadowMap;

uniform vec3 materialCoefficients; // x = ambient, y = diffuse, z = specular 
uniform float specularAlpha;

uniform samplerCube skybox;

uniform bool draw_texcoords;
uniform bool draw_normals;
uniform bool gamma;

uniform struct DirectionalLight {
	vec3 color;
	vec3 direction;
} dirL;

uniform struct PointLight {
	vec3 color;
	vec3 position;
	vec3 attenuation;
} pointL;

vec3 phong(vec3 n, vec3 l, vec3 v, vec3 diffuseC, float diffuseF, vec3 specularC, float specularF, float alpha, bool attenuate, vec3 attenuation) {
	float d = length(l);
	l = normalize(l);
	float att = 1.0;	
	if (attenuate) {
        if (gamma) {
            att = 1.0 / (attenuation.x + d * attenuation.y + d * d * attenuation.z);
        } else {
            att = 1.0 / sqrt(attenuation.x + d * attenuation.y + d * d * attenuation.z);
        }
    }
	vec3 r = reflect(-l, n);
	return (diffuseF * diffuseC * max(0, dot(n, l)) + specularF * specularC * pow(max(0, dot(r, v)), alpha)) * att; 
}

float computeFresnelReflectance(float reflectionCoefficient, float cosTheta) {
    float r0 = reflectionCoefficient;
    return r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
}

float ShadowCalculation(float dotLightNormal)
{
    vec3 pos = FragPosLightSpace.xyz * 0.5 + 0.5;
    if(pos.z > 1.0){
        pos.z = 1.0;
    }
    float depth = texture(shadowMap, pos.xy).r;
    float bias = max(0.05 * (1.0 - dotLightNormal), 0.005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float depth = texture(shadowMap, pos.xy + vec2(x, y) * texelSize).r; 
            shadow += (depth + bias) < pos.z ? 0.3 : 1.0;    
        }    
    }
    return shadow / 9.0;
 }

void main() {
    vec3 n = normalize(out_normals);
	vec3 v = normalize(position_world - camera_world);
	
	vec3 F0 = vec3(0.1); // <-- some kind of plastic

	float cosTheta = abs(dot(-v, n));
	float reflectionCoefficient = 1;
	float reflectance = computeFresnelReflectance(reflectionCoefficient, cosTheta);

	vec3 texColor = texture(texture_diffuse, TexCoords).rgb;
	vec3 ambient = texColor * materialCoefficients.x; // ambient
	
	// add directional light contribution
	vec3 direct = phong(n, -dirL.direction, -v, dirL.color * texColor, materialCoefficients.y, dirL.color, materialCoefficients.z, specularAlpha, false, vec3(0));
			
	// add point light contribution
	vec3 point = phong(n, pointL.position - position_world, -v, pointL.color * texColor, materialCoefficients.y, pointL.color, materialCoefficients.z, specularAlpha, true, pointL.attenuation);

	vec3 I = normalize(position_world - camera_world);
    vec3 R = reflect(I, normalize(n));
	vec3 reflectedColor = texture(skybox, R).rgb;
	texColor = mix(texColor, reflectedColor, 0.1f); //hier ändern wenn du mehr reflektion willst

	float dotLightNormal = dot(-dirL.direction, n);

	// calculate shadow
    float shadow = ShadowCalculation(dotLightNormal); 

	texColor = (ambient + ((shadow) * (direct + point))) * texColor;

	if(gamma){
        texColor = pow(texColor, vec3(1.0/2.2));
	}
	// check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(texColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(texColor, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

	 color = vec4(texColor, 1.0);

	if (draw_normals) {
		color.rgb = n;
	}
	if (draw_texcoords) {
		color.rgb = vec3(TexCoords, 0);
	}
}
