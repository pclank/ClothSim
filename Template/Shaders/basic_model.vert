#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    TexCoords = aTexCoords;
	
	vec3 fragPos = vec3(model * vec4(aPos, 1.0));
	FragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
	
    gl_Position = projection * view * vec4(fragPos, 1.0f);
}

