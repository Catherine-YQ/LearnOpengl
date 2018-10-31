#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


const GLuint WIDTH = 1280, HEIGHT = 720;
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

glm::vec3 lightPos(-2, 2.0, -2);   //��Դλ��
glm::vec3 cubePos(0.0, 0, -5.0);   //������������λ��
glm::vec3 groundPos(1, -1, -6);    //����λ��
glm::vec3 groundScale(7, 1.0, 7);  //��չ����
glm::vec3 cameraPos(0, 4, 0);      //���λ��


bool initOpengl(int width, int height, GLFWwindow** ppWindow);    //��ʼ��opengl
void renderCube();                                               
void renderQuad();
GLuint createTexture(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type);
GLuint createFramerbuffer(int colorTex, int depthTex);
void shadowPass(Shader& shader,GLuint shadowMapFbo);                                          //������Ӱ��ͼ
void blurPass(Shader& shader, GLuint blurFbo, GLuint blurColorTexture, GLuint shadowMpFbo,GLuint shadowMapColorTexture); //����Ӱ��ͼ�����˲�
void scenePass(Shader& shader,GLuint shadowMapColorTexture);                                 //������Ⱦ



int main()
{
	GLFWwindow *window = nullptr;
	initOpengl(WIDTH, HEIGHT, &window);
	
	Shader shadowShader("shadowMap.vs", "shadowMap.frag");
	Shader blurShader("blur.vs", "blur.frag");
	Shader sceneShader("shadow.vs", "shadow.frag");

	//���ڲ�����Ӱ��ͼ��shadowMapColorTexture�洢ƬԴ�����Դ��depth���Լ�depth*depth
	GLuint shadowMapDepthTexture = createTexture(GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT);
	GLuint shadowMapColorTexture = createTexture(GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RG, GL_FLOAT);
	GLuint shadowMapFbo = createFramerbuffer(shadowMapColorTexture, shadowMapDepthTexture);

	//���ڶ���Ӱ��ͼ�˲���ֻ��Ҫ������ͼ����
	GLuint blurColorTexture = createTexture(GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RG, GL_FLOAT);
	GLuint blurFbo = createFramerbuffer(blurColorTexture, -1);


	glEnable(GL_DEPTH_TEST);
	

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		shadowPass(shadowShader,shadowMapFbo);                                               //������Ӱ��ͼ
		blurPass(blurShader, blurFbo, blurColorTexture, shadowMapFbo,shadowMapColorTexture); //ģ����Ӱ��ͼ
		scenePass(sceneShader, shadowMapColorTexture);                                       //������Ӱ
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}


bool initOpengl(int width, int height, GLFWwindow** ppWindow)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", nullptr, nullptr); 
	if (!window)
	{
		std::cerr << "opengl init error" << __FILE__ << " " << __LINE__ << std::endl;
		return 0;
	}
	glfwMakeContextCurrent(window);

	//glfwSetKeyCallback(window, key_callback);
	*ppWindow = window;


	glewExperimental = GL_TRUE;
	glewInit();
	return true;
}

GLuint cubeVao;
void renderCube()
{
	if (cubeVao == 0){
		float cubeVertices[] = {
			// Front-face
			// Pos              // Color          //Tex       // Norm       
			-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top-left
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top-right
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Bottom-right
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Bottom-right
			-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Bottom-left
			-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top-left

			// Left-side-face
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Top-left
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Top-right
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, //Bottom-right
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, //Bottom-right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, //Bottom-left
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, //Top-left.
			// Right-side-face
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Left-top
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Right-top
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, //Bottom-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, //Bottom-right
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, //Bottom-left
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Left-top

			// Top-face
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Front-left
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Back-left
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Back-right
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Back-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Front-right
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Front-left

			// Bottom-face
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, // Front-left
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f, // Front-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, // Back-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, // Back-right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, // Back-left
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, // Front-left

			// Back-face
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, // Top-left
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f, // Bottom-left
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, // Bottom-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, // Bottom-right
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, // Top-right
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, // Top-left
		};

		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);
		// Color
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
		// Texcoords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
		// Normal
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	}
	glBindVertexArray(cubeVao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

GLuint quadVao;
void renderQuad()
{
	if (quadVao == 0)
	{
		float quadVertices[] = {
			// Front-face
			// Pos              // Color          //Tex       // Norm       
			-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top-left
			1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top-right
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Bottom-right

			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Bottom-right
			-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, //Bottom-left
			-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top-left
		};
		GLuint vbo;
		glGenVertexArrays(1, &quadVao);
		glBindVertexArray(quadVao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
		glBindVertexArray(0);
	}
	glBindVertexArray(quadVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

GLuint createTexture(GLint internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type)
{
	GLuint tex;
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;

}

GLuint createFramerbuffer(int colorTex, int depthTex)
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (depthTex != -1)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
	if (colorTex != -1)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != result) {
		std::cerr<<"ERROR: Framebuffer is not complete.\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}



void shadowPass(Shader& shader,GLuint shadowMapFbo)
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//here is ok without depthbuffer because color(depth,depth*depth,0,0)is only needed
	shader.Use();
	glm::mat4 mat;
	mat *= glm::perspective(45.0f, 1.0f, 2.0f, 100.0f);
	mat *= glm::lookAt(lightPos, cubePos, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "cameraToShadowProjector"), 1, GL_FALSE, glm::value_ptr(mat));

	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), cubePos)));
	renderCube();
	//render ground
	glm::mat4 model = glm::translate(glm::mat4(), groundPos);
	model = glm::scale(model, groundScale);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	renderCube();
}


//main:
//shadowPass(shadowShader, shadowMapFbo);                                               //������Ӱ��ͼ
//blurPass(blurShader, blurFbo, blurColorTexture, shadowMapFbo, shadowMapColorTexture); //ģ����Ӱ��ͼ
//scenePass(sceneShader, shadowMapColorTexture);                                       //������Ӱ

//GLuint shadowMapDepthTexture = createTexture(GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT);
// shadowMapColorTexture = createTexture(GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RG, GL_FLOAT);
//GLuint shadowMapFbo = createFramerbuffer(shadowMapColorTexture, shadowMapDepthTexture);

//���ڶ���Ӱ��ͼ�˲���ֻ��Ҫ������ͼ����
//GLuint blurColorTexture = createTexture(GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RG, GL_FLOAT);
//GLuint blurFbo = createFramerbuffer(blurColorTexture, -1);

void blurPass(Shader& shader, GLuint blurFbo, GLuint blurColorTexture, GLuint shadowMpFbo, GLuint shadowMapColorTexture)
{
	shader.Use();
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glDisable(GL_DEPTH_TEST);

	//����x������ģ��
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
	glBindTexture(GL_TEXTURE_2D, shadowMapColorTexture);
	glUniform2f(glGetUniformLocation(shader.Program, "ScaleU"),1.0/SHADOW_WIDTH*2.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	//��y������ģ��
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);//shadowMpFbo);
	glBindTexture(GL_TEXTURE_2D, blurColorTexture);
	glUniform2f(glGetUniformLocation(shader.Program, "ScaleU"), 0.0, 1.0 / SHADOW_HEIGHT*2.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void scenePass(Shader& shader, GLuint shadowMapColorTexture)
{
	shader.Use();
	glViewport(0, 0, WIDTH, HEIGHT);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 proj = glm::perspective((float)45, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0, 1, -1), glm::vec3(0, 1, 0));
	glm::mat4 mat;
	mat *= glm::perspective(45.0f, 1.0f, 2.0f, 100.0f);
	mat *= glm::lookAt(lightPos, cubePos, glm::vec3(0, 1, 0));

	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "cameraToShadowProjector"), 1, GL_FALSE, glm::value_ptr(mat));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
	glUniform3f(glGetUniformLocation(shader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glBindTexture(GL_TEXTURE_2D, shadowMapColorTexture);

	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), cubePos)));
	renderCube();

	glm::mat4 model = glm::translate(glm::mat4(), groundPos);
	model = glm::scale(model, groundScale);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	renderCube();

}
