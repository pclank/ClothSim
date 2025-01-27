#version 430

uniform vec3 baseColor;

out vec4 outColor;

void main()
{
    outColor = vec4(baseColor, 1.0);
}
