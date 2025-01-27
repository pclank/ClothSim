#version 430

in vec3 TestColor;

out vec4 outColor;

void main()
{
    outColor = vec4(TestColor, 1.0);
}
