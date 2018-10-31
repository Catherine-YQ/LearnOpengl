#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

out vec4 normalInViewSpace;
out vec4 posInViewSpace;
out vec4 posInLightSpace;
out vec2 Texcoord;

uniform mat4 lightSpaceMatrix;
uniform mat4 model, view, proj;

void main() {
	Texcoord = texcoord;
	gl_Position = proj * view * model * vec4(position, 1.0f);
	normalInViewSpace = view * model * vec4(normal,   0.0f);
	posInViewSpace = view * model * vec4(position, 1.0);
	posInLightSpace = lightSpaceMatrix * model * vec4(position, 1.0f);
}