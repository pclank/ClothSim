#version 430
// *****************************************************
// Shader used for the Skybox
// *****************************************************

layout (location = 0) in vec3 vPos;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 TexCoords;

void main()
{
    TexCoords = vPos;
    gl_Position = (projectionMatrix * viewMatrix * vec4(vPos, 1.0)).xyww;
}  