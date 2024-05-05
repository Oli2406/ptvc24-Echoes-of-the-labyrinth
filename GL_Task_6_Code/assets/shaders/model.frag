#version 430 core

in vec3 out_normals;
in vec3 position_world;
in vec2 TexCoords;

out vec4 color;

uniform vec3 camera_world;
uniform sampler2D texture_diffuse;
uniform vec3 materialCoefficients; // x = ambient, y = diffuse, z = specular 
uniform float specularAlpha;

uniform bool draw_texcoords;
uniform bool draw_normals;

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
	if(attenuate) att = 1.0f / (attenuation.x + d * attenuation.y + d * d * attenuation.z);
	vec3 r = reflect(-l, n);
	return (diffuseF * diffuseC * max(0, dot(n, l)) + specularF * specularC * pow(max(0, dot(r, v)), alpha)) * att; 
}

void main() {
    vec3 n = normalize(out_normals);
	vec3 v = normalize(position_world - camera_world);
	
	vec3 F0 = vec3(0.1); // <-- some kind of plastic
	

	vec3 texColor = texture(texture_diffuse, TexCoords).rgb;
	color = vec4(texColor * materialCoefficients.x, 1); // ambient
	
	// add directional light contribution
	color.rgb += phong(n, -dirL.direction, -v, dirL.color * texColor, materialCoefficients.y, dirL.color, materialCoefficients.z, specularAlpha, false, vec3(0));
			
	// add point light contribution
	color.rgb += phong(n, pointL.position - position_world, -v, pointL.color * texColor, materialCoefficients.y, pointL.color, materialCoefficients.z, specularAlpha, true, pointL.attenuation);

	color = vec4(color.xyz, 1.0);

	if (draw_normals) {
		color.rgb = n;
	}
	if (draw_texcoords) {
		color.rgb = vec3(TexCoords, 0);
	}
}