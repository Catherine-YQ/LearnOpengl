

#version 330 core
out vec4 FragColor;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 1

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in vec4 posInLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D shadowMap;
uniform float shininess;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
//uniform Material material;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadowFactor);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


float chebyshevUpperBound(float distance,vec2 texCoord)
{
	vec2 moments = texture(shadowMap,texCoord).rg;
	
	//片元不在阴影里
	if (distance <= moments.x)
		return 1.0;
	//计算片元在阴影里的最大概率上界
	float variance = moments.y - (moments.x*moments.x);
	variance = max(variance, 0.00002);

	float d = distance - moments.x;
	float p_max = variance / (variance + d*d);

	return p_max;
}

void main()
{    
    // properties
    //vec3 norm = normalize(Normal);
	vec3 norm = texture(texture_normal1,TexCoords).rgb;
	norm = normalize(norm * 2.0 - 1.0);
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);

    vec4 lightTexCoord = posInLightSpace / posInLightSpace.w; 
	lightTexCoord = lightTexCoord * 0.5 + 0.5;

	float shadowFactor = 0; 
	//判断是否超出了边界
	bool outsideShadowMap = posInLightSpace.w <= 0.0f || (lightTexCoord.x < 0 || lightTexCoord.y < 0) || (lightTexCoord.x >= 1 || lightTexCoord.y >= 1);
	if (!outsideShadowMap) 
	{
		shadowFactor = chebyshevUpperBound(lightTexCoord.z,lightTexCoord.xy);
	}
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir, shadowFactor);
    // phase 2: point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    //result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
    // phase 3: spot light
    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);

    //FragColor = texture(texture_specular1,TexCoords);
    //FragColor = vec4(1.0,1.0,1.0,1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadowFactor)
{
    //vec3 lightDir = normalize(-TBN*light.direction);
    // diffuse shading
	vec3 lightDir = normalize(-TangentLightPos);
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords))*shadowFactor;

    vec3 specular =light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    return (ambient + diffuse+ specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}