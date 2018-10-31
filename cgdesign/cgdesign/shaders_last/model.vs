

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

out vec4 posInLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix*tangent);
    vec3 B = normalize(normalMatrix*bitangent);
    vec3 N = normalize(normalMatrix*aNormal);

    mat3 TBN = transpose(mat3(T, B, N));  
    TangentLightPos = TBN * lightPos;//方向光怎么办？
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;

	posInLightSpace = lightSpaceMatrix * model * vec4(aPos, 1.0f);
}

