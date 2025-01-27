#version 430
// *****************************************************
// Shader used for the Skybox
// *****************************************************

in vec3 TexCoords;

uniform samplerCube skybox;

out vec4 FragColor;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}