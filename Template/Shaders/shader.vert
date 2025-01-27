#version 430

layout (location = 0) in vec3 vPos;

uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vPos, 1.0);
//	gl_Position = vec4(vPos, 1.0);
}