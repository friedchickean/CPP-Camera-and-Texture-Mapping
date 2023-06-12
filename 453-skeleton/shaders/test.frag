#version 330 core

in vec3 fragPos;
in vec3 fragColor;
in vec3 n;
in vec2 tc;

uniform vec3 light;
uniform sampler2D sampler;

out vec4 color;

void main() {
	vec4 d = texture(sampler, tc);

	vec3 lightDir = normalize(light - fragPos);
    vec3 normal = normalize(n);
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

    float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;

	float ambientStrength = 0.04;
	vec3 ambient = ambientStrength * lightColor;

	vec3 result = (ambient + diffuse);
	color = vec4(result, 1.0) * d;

}
