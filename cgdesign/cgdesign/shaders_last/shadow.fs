#version 330

in vec4 posInViewSpace;
in vec4 normalInViewSpace;
in vec2 Texcoord;
in vec4 posInLightSpace;

out vec4 outColor;

uniform sampler2D shadowMap;

uniform mat4 view;
uniform vec3 lightPos;

float chebyshevUpperBound(float distance,vec2 texCoord)
{
	vec2 moments = texture(shadowMap,texCoord).rg;
	
	//片元不在阴影里
	if (distance <= moments.x)
		return 1.0;
	//计算片元在阴影里的最大概率上届
	float variance = moments.y - (moments.x*moments.x);
	variance = max(variance, 0.00002);

	float d = distance - moments.x;
	float p_max = variance / (variance + d*d);

	return p_max;
}

void main() 
{
	vec3 fragment = vec3(posInViewSpace);
	vec3 normal   = vec3(normalize(normalInViewSpace));
	vec3 viewDir  = normalize(-fragment); //视线方向

	//计算片元在shadowMap里的对应位置
	vec4 lightTexCoord = posInLightSpace / posInLightSpace.w; 
	lightTexCoord = lightTexCoord * 0.5 + 0.5;

	float shadowFactor = 1.0; 
	//判断是否超出了边界
	bool outsideShadowMap = posInLightSpace.w <= 0.0f || (lightTexCoord.x < 0 || lightTexCoord.y < 0) || (lightTexCoord.x >= 1 || lightTexCoord.y >= 1);
	if (!outsideShadowMap) 
	{
		shadowFactor = chebyshevUpperBound(lightTexCoord.z,lightTexCoord.xy);
	}

	
	//把片元从世界坐标系下变换到ViewSpace
	vec3 lightInViewSpace = vec3(view * vec4(lightPos, 1.0));

	vec4 materialDiffColor = vec4(1,1,1,1); //物体的漫反射颜色

	vec3 positionToLight = lightInViewSpace - fragment;
	vec3 lightDir  = normalize(positionToLight);

	//光线和法向量的夹角
	float cosAngIncidence = dot(lightDir, normal);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);


	vec4 diffuse  = materialDiffColor * vec4(1.0,1.0,1.0,1.0) * cosAngIncidence*shadowFactor;   //计算最后的漫反射颜色
	vec4 ambient = vec4(0.1, 0.1, 0.1, 1.0) * materialDiffColor; //环境光颜色
	vec4 color = ambient+diffuse; //最后的颜色

	outColor = vec4(vec3(color), 1.0);
};