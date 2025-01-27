#version 400 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
// uniform sampler2D texture_specular1;
// uniform sampler2D texture_height1;
uniform sampler2D shadowMap;				// Direction shadows
uniform samplerCube shadowCubemap;			// Omnidirectional shadows

uniform vec3 lightPos;
uniform vec3 pointLightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform float minBias;
uniform float maxBias;
uniform float bias;
uniform int shadowSamples;
uniform float farPlane;
uniform bool directionalShadows;
uniform bool omniShadows;
uniform bool normalMapOn;

vec3 GetNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal1, fs_in.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.FragPos);
    vec3 Q2  = dFdy(fs_in.FragPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N   = normalize(fs_in.Normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
	
	if(projCoords.z > 1.0f)
        return 0.0f;
	
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).x; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

    return shadow;
}

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCubemapCalculation(vec3 fragPos, float bias)
{
	// Sample the depth map
	
	vec3 fragToLight = fragPos - pointLightPos; 
    // float closestDepth = texture(shadowCubemap, fragToLight).r;
	// closestDepth *= farPlane;
	
	// Get current linear depth as the length between the fragment and light position
	float currentDepth = length(fragToLight);
	// float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
	
	// float shadow  = 0.0f;
	// float samples = 4.0f;
	// float offset  = 0.1f;
	// for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	// 	for(float y = -offset; y < offset; y += offset / (samples * 0.5))
	// 		for(float z = -offset; z < offset; z += offset / (samples * 0.5))
	// 		{
	// 			float closestDepth = texture(shadowCubemap, fragToLight + vec3(x, y, z)).r; 
	// 			closestDepth *= farPlane;   // undo mapping [0;1]
	// 			if(currentDepth - bias > closestDepth)
	// 				shadow += 1.0;
	// 		}
	// 
	// shadow /= (samples * samples * samples);
	
	float shadow = 0.0f;
    int samples = shadowSamples;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0f + (viewDistance / farPlane)) / 25.0f;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadowCubemap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0f;
    }
    shadow /= float(samples);
	
	return shadow;
}

vec3 CalculateDirectionalLight(vec3 color, vec3 normal)
{
	// ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
	
	// Calculate shadow
	// float calcBias = max(0.05f * (1.0f - dot(normal, lightDir)), 0.005f);
	float calcBias = max(maxBias * (1.0f - dot(normal, lightDir)), minBias);
	float directionalShadow = (directionalShadows) ? ShadowCalculation(fs_in.FragPosLightSpace, calcBias) : 0.0f;
	
	return (ambient + (1.0f - directionalShadow) * (diffuse + specular)) * color;
}

vec3 CalculatePointLight(vec3 color, vec3 normal)
{
	// ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(pointLightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
	
	// Calculate shadow
	float omniShadow = (omniShadows) ? ShadowCubemapCalculation(fs_in.FragPos, bias) : 0.0f;
	
	return (ambient + (1.0f - omniShadow) * (diffuse + specular)) * color;
}

void main()
{           
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 normal = (normalMapOn) ? GetNormalFromMap() : normalize(fs_in.Normal);
	
	vec3 lighting = vec3(0.0f);

	// vec3 lighting = vec3(omniShadow / farPlane);
	
	//float totalShadow = (omniShadows && directionalShadows) ? (directionalShadow + omniShadow) * 0.5f :
	//	(omniShadows) ? omniShadow : directionalShadow;
	// lighting = (ambient + (1.0f - omniShadow) * (diffuse + specular)) * color;
	// lighting = (ambient + (1.0f - totalShadow) * (diffuse + specular)) * color;
	
	lighting = CalculateDirectionalLight(color, normal) + CalculatePointLight(color, normal);
    
    FragColor = vec4(lighting, 1.0);
}