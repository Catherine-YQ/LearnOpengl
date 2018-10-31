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
	
	//ƬԪ������Ӱ��
	if (distance <= moments.x)
		return 1.0;
	//����ƬԪ����Ӱ����������Ͻ�
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
	vec3 viewDir  = normalize(-fragment); //���߷���

	//����ƬԪ��shadowMap��Ķ�Ӧλ��
	vec4 lightTexCoord = posInLightSpace / posInLightSpace.w; 
	lightTexCoord = lightTexCoord * 0.5 + 0.5;

	float shadowFactor = 1.0; 
	//�ж��Ƿ񳬳��˱߽�
	bool outsideShadowMap = posInLightSpace.w <= 0.0f || (lightTexCoord.x < 0 || lightTexCoord.y < 0) || (lightTexCoord.x >= 1 || lightTexCoord.y >= 1);
	if (!outsideShadowMap) 
	{
		shadowFactor = chebyshevUpperBound(lightTexCoord.z,lightTexCoord.xy);
	}

	
	//��ƬԪ����������ϵ�±任��ViewSpace
	vec3 lightInViewSpace = vec3(view * vec4(lightPos, 1.0));

	vec4 materialDiffColor = vec4(1,1,1,1); //�������������ɫ

	vec3 positionToLight = lightInViewSpace - fragment;
	vec3 lightDir  = normalize(positionToLight);

	//���ߺͷ������ļн�
	float cosAngIncidence = dot(lightDir, normal);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);


	vec4 diffuse  = materialDiffColor * vec4(1.0,1.0,1.0,1.0) * cosAngIncidence*shadowFactor;   //����������������ɫ
	vec4 ambient = vec4(0.1, 0.1, 0.1, 1.0) * materialDiffColor; //��������ɫ
	vec4 color = ambient+diffuse; //������ɫ

	outColor = vec4(vec3(color), 1.0);
};