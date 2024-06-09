

#version 430 core
/*
* Copyright 2023 Vienna University of Technology.
* Institute of Computer Graphics and Algorithms.
* This file is part of the GCG Lab Framework and must not be redistributed.
*/

in VertexData {
	vec3 position_world;
	vec3 normal_world;
	vec2 uv;
} vert;

in vec4 FragPosLightSpace;

out vec4 color;

uniform vec3 camera_world;

uniform vec3 materialCoefficients; // x = ambient, y = diffuse, z = specular 
uniform float specularAlpha;
uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform bool draw_normals;
uniform bool draw_texcoords;
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
            att = 1.0 / (attenuation.x + d * d * attenuation.y + d * d * d * d * attenuation.z);
        } else {
            att = 1.0 / (attenuation.x + d * attenuation.y + d * d * attenuation.z);
        }
    }
	vec3 r = reflect(-l, n);
	return (diffuseF * diffuseC * max(0, dot(n, l)) + specularF * specularC * pow(max(0, dot(r, v)), alpha)) * att; 
}

// Gets the reflected color value from a a certain position, from 
// a certain direction INSIDE of a cornell box of size 3 which is 
// positioned at the origin.
// positionWS:  Position inside the cornell box for which to get the
//              reflected color value for from -directionWS.
// directionWS: Outgoing direction vector (from positionWS towards the
//              outside) for which to get the reflected color value for.
vec3 getCornellBoxReflectionColor(vec3 positionWS, vec3 directionWS) {
	vec3 P0 = positionWS;
	vec3 V  = normalize(directionWS);

	const float boxSize = 1.5;
	vec4[5] planes = {
		vec4(-1.0,  0.0,  0.0, -boxSize), // left
		vec4( 1.0,  0.0,  0.0, -boxSize), // right
		vec4( 0.0,  1.0,  0.0, -boxSize), // top
		vec4( 0.0, -1.0,  0.0, -boxSize), // bottom
		vec4( 0.0,  0.0, -1.0, -boxSize)  // back
	};
	vec3[5] colors = {
		vec3(1.0, 0.0, 0.0),    // left
		vec3(0.0, 1.0, 0.0),    // right
		vec3(0.96, 0.93, 0.85), // top
		vec3(0.64, 0.64, 0.64), // bottom
		vec3(0.76, 0.74, 0.68)  // back
	};

	for (int i = 0; i < 5; ++i) {
		vec3  N = planes[i].xyz;
		float d = planes[i].w;
		float denom = dot(V, N);
		if (denom <= 0) continue;
		float t = -(dot(P0, N) + d)/denom;
		vec3  P = P0 + t*V;
		float q = boxSize + 0.01;
		if (P.x > -q && P.x < q && P.y > -q && P.y < q && P.z > -q && P.z < q) {
			return colors[i];
		}
	}
	return vec3(0.0, 0.0, 0.0);
}

// Computes the reflection direction for an incident vector I about normal N, 
// and clamps the reflection to a maximum of 180, i.e. the reflection vector
// will always lie within the hemisphere around normal N.
// Aside from clamping, this function produces the same result as GLSL's reflect function.
vec3 clampedReflect(vec3 I, vec3 N)
{
	return I - 2.0 * min(dot(N, I), 0.0) * N;
}

vec3 fresnelSchlick (float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow (1.0 - cosTheta, 5.0);
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
            shadow += (depth + bias) < pos.z ? 0.0 : 1.0;    
        }    
    }
    return shadow / 9.0;
 }

void main() {	
	vec3 n = normalize(vert.normal_world);
	vec3 v = normalize(vert.position_world - camera_world);
	//vec3 R = normalize(clampedReflect(v, n));
	//vec3 reflectionColor = getCornellBoxReflectionColor(vert.position_world, R);
	vec3 F0 = vec3(0.1); // <-- some kind of plastic
	//vec3 reflectivity = fresnelSchlick(dot(n, -v), F0);

	vec3 texColor = texture(diffuseTexture, vert.uv).rgb;
	vec3 ambient = texColor * materialCoefficients.x; // ambient
	
	// add directional light contribution
	vec3 direct = phong(n, -dirL.direction, -v, dirL.color * texColor, materialCoefficients.y, dirL.color, materialCoefficients.z, specularAlpha, false, vec3(0));
			
	// add point light contribution
	vec3 point = phong(n, pointL.position - vert.position_world, -v, pointL.color * texColor, materialCoefficients.y, pointL.color, materialCoefficients.z, specularAlpha, true, pointL.attenuation);

	//color = vec4(mix(color.xyz, reflectionColor, reflectivity), 1.0f);

	float dotLightNormal = dot(-dirL.direction, n);

	// calculate shadow
    float shadow = ShadowCalculation(dotLightNormal); 

	texColor = (ambient + ((shadow) * (direct + point))) * texColor;

	if(gamma){
        texColor = pow(texColor, vec3(1.0/2.2));
	}
	color = vec4(texColor, 1.0);

	//color = vec4(color.xyz, 1);

	if (draw_normals) {
		color.rgb = n;
	}
	if (draw_texcoords) {
		color.rgb = vec3(vert.uv, 0);
	}
}
