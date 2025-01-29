#version 400 core

in vec2 fragTexCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse;

uniform vec3 lightPos;
uniform vec3 pointLightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
	vec3 lighting = vec3(0.0f);
	// lighting = CalculateDirectionalLight(color, normal) + CalculatePointLight(color, normal);

	// FragColor = vec4(1.0f, 0.0f, 0.5f, 1.0f);
	
	vec3 color = texture(texture_diffuse, fragTexCoords).rgb;
	
	// FragColor = vec4(fragTexCoords.x, 0.0f, fragTexCoords.y, 1.0f);
	FragColor = vec4(color, 1.0f);
}