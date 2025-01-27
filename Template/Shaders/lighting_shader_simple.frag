#version 430

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 outColor;

uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform vec3 CamPos;    
uniform vec3 LightPosition;
uniform vec3 BaseColor;
uniform vec3 ManualLightColor;
uniform float ManualMetallic;
uniform float ManualRoughness;

float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

void main()
{	
    //hardcoded for now
    float metallic = ManualMetallic; 
    float roughness = ManualRoughness; 
    vec3 albedo = BaseColor;       //determines the colour
    //vec3 LightPosition = vec3(10.0f, 10.0f, 10.0f);      //TODO: change with correct calculations and do a for loop
    vec3 LightColor = ManualLightColor;
    vec3 N = normalize(Normal);
    vec3 V = normalize(CamPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    
    // calculate per-light radiance
    vec3 L = normalize(LightPosition - WorldPos);
    vec3 H = normalize(V + L);
    float l_distance    =  length(LightPosition - WorldPos);
    float attenuation = 1.0 / (l_distance * l_distance);
    vec3 radiance     = LightColor * attenuation;        
        
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
            
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);                
    Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    
    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    outColor = vec4(color, 1.0);
}  