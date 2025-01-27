#version 400 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).x; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - 0.005f > closestDepth  ? 1.0 : 0.0;

    return shadow;
}  

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords) * (1.0f - ShadowCalculation(FragPosLightSpace));
	// FragColor = FragPosLightSpace;
}

