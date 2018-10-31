#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>


#include <opengl/shader.h>
#include <opengl/camera.h>
#include <opengl/model.h>
#include <opengl/mesh.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

void renderQuad();
GLuint createTexture(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type);
GLuint createFramerbuffer(int colorTex, int depthTex);
void shadowPass(Shader& shader, GLuint shadowMapFbo, Model ourModel);                                          //产生阴影贴图
void blurPass(Shader& shader, GLuint blurFbo, GLuint blurColorTexture, GLuint shadowMpFbo, GLuint shadowMapColorTexture); //对阴影贴图进行滤波
void scenePass(Shader& shader, GLuint shadowMapColorTexture, Model ourModel);
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 pointLightPositions[] = {
	glm::vec3(1.3f,  1.3f,  0.0f),
	glm::vec3(1.3f, -1.3f, 0.0f),
	glm::vec3(4.0f,  5.0f, -10.0f),
	glm::vec3(3.0f, -2.0f, 4.0f)
};

glm::vec3 boatPos(0.0f, -2.0f, 0.0f);
glm::vec3 lightPos(0.2f, 1.0f, 0.3f);

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

														 // glfw window creation
														 // --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	//glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("shaders_last/model.vs","shaders_last/model.fs");//光照与法线贴图与最后的阴影贴图
	Shader shadowShader("shaders_last/shadowMap.vs", "shaders_last/shadowMap.fs");//用VSM
	Shader blurShader("shaders_last/blur.vs", "shaders_last/blur.fs");//滤波混合
	//Shader sceneShader("shadow.vs", "shadow.fs");//shadow shader
	// load models
	// -----------
	Model ourModel("resources/boat/Boat.obj");

	GLuint shadowMapDepthTexture = createTexture(GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT, GL_FLOAT);
	GLuint shadowMapColorTexture = createTexture(GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RG, GL_FLOAT);
	GLuint shadowMapFbo = createFramerbuffer(shadowMapColorTexture, shadowMapDepthTexture);

	GLuint blurColorTexture = createTexture(GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT, GL_RG, GL_FLOAT);
	GLuint blurFbo = createFramerbuffer(blurColorTexture, -1);

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_DEPTH_TEST);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		//glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		shadowPass(shadowShader, shadowMapFbo,ourModel);                                               //产生阴影贴图
		blurPass(blurShader, blurFbo, blurColorTexture, shadowMapFbo, shadowMapColorTexture); //模糊阴影贴图
		scenePass(ourShader, shadowMapColorTexture,ourModel);


		// don't forget to enable shader before setting uniform

		//ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		//ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		//ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		//ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		//ourShader.setFloat("pointLights[0].constant", 1.0f);
		//ourShader.setFloat("pointLights[0].linear", 0.09);
		//ourShader.setFloat("pointLights[0].quadratic", 0.032);
		// view/projection transformations
		//试试如何整合shader
		//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		//glm::mat4 view = camera.GetViewMatrix();
		//ourShader.setMat4("projection", projection);
		//ourShader.setMat4("view", view);

		//// render the loaded model
		//glm::mat4 model=glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f)); // translate it down so it's at the center of the scene
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		////model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		//ourShader.setMat4("model", model);

		// directional light


		//ourModel.Draw(ourShader);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


//生成并设置自己的纹理（不从模型的）
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

//生成framebuffer
GLuint createFramerbuffer(int colorTex, int depthTex)
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (depthTex != -1)//生成depthMap
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
	if (colorTex != -1)//生成颜色Map 这里只是用其中2个通道来做算法
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != result) {
		std::cerr << "ERROR: Framebuffer is not complete.\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return fbo;
}

void shadowPass(Shader& shader, GLuint shadowMapFbo, Model ourModel)
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFbo);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader.use();
	glm::mat4 mat = glm::mat4(1.0f);
	GLfloat near_plane = 1.0f, far_plane = 7.5f;
	mat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	mat *= glm::lookAt(lightPos, boatPos, glm::vec3(0.0, 1.0, 0.0));
	//glUniformMatrix4fv(glGetUniformLocation(shader.Program, "cameraToShadowProjector"), 1, GL_FALSE, glm::value_ptr(mat));
	shader.setMat4("lightSpaceMatrix", mat);//改了uniform的名 记得改回来
	//glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), cubePos)));
	//renderCube();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, boatPos); // translate it down so it's at the center of the scene
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	shader.setMat4("model", model);
	ourModel.Draw(shader,-1);//大概是这样吧，此时正在生成深度贴图，不用绑其他纹理
	//glm::mat4 model = glm::translate(glm::mat4(), groundPos);
	//model = glm::scale(model, groundScale);
	//glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	//renderCube();
}

void blurPass(Shader& shader, GLuint blurFbo, GLuint blurColorTexture, GLuint shadowMpFbo, GLuint shadowMapColorTexture)
{
	shader.use();
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glDisable(GL_DEPTH_TEST);//渲染帧缓冲不需要深度测试

	//先在x方向上模糊
	glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
	glBindTexture(GL_TEXTURE_2D, shadowMapColorTexture);
	//glUniform2f(glGetUniformLocation(shader.Program, "ScaleU"), 1.0 / SHADOW_WIDTH * 2.0, 0.0);
	shader.setVec2("ScaleU", 1.0 / SHADOW_WIDTH * 2.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();
	//x方向模糊完了以后相当于用blurFBO的blurColorTexture存储起来了
	//后y方向上模糊
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMpFbo);
	glBindTexture(GL_TEXTURE_2D, blurColorTexture);//沿用上一步x方向的模糊结果作为基础
	//glUniform2f(glGetUniformLocation(shader.Program, "ScaleU"), 0.0, 1.0 / SHADOW_HEIGHT * 2.0);
	shader.setVec2("ScaleU", 0.0, 1.0 / SHADOW_HEIGHT * 2.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();
	//y方向也模糊完了，相当于用shadowMapFbo的shadowMapColorTexture存储起来了总的结果
	glBindFramebuffer(GL_FRAMEBUFFER, 0);//返回默认帧缓冲
	glEnable(GL_DEPTH_TEST);
}

void scenePass(Shader& shader, GLuint shadowMapColorTexture,Model ourModel)
{
	shader.use();
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glm::mat4 proj = glm::perspective((float)45, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	//glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0, 1, -1), glm::vec3(0, 1, 0));
	//glm::mat4 mat;
	//mat *= glm::perspective(45.0f, 1.0f, 2.0f, 100.0f);
	//mat *= glm::lookAt(lightPos, cubePos, glm::vec3(0, 1, 0));

	shader.setVec3("viewPos", camera.Position);
	shader.setFloat("shininess", 32.0f);
	shader.setVec3("dirLight.direction", lightPos);
	shader.setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
	shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

	glm::mat4 mat = glm::mat4(1.0f);
	GLfloat near_plane = 1.0f, far_plane = 7.5f;
	mat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	mat *= glm::lookAt(lightPos, boatPos, glm::vec3(0.0, 1.0, 0.0));
	//glUniformMatrix4fv(glGetUniformLocation(shader.Program, "cameraToShadowProjector"), 1, GL_FALSE, glm::value_ptr(mat));
	shader.setMat4("lightSpaceMatrix", mat);//改了uniform的名 记得改回来

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
	shader.setMat4("model", model);
	shader.setVec3("lightPos", lightPos);
	//glUniformMatrix4fv(glGetUniformLocation(shader.Program, "cameraToShadowProjector"), 1, GL_FALSE, glm::value_ptr(mat));
	//glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	//glUniformMatrix4fv(glGetUniformLocation(shader.Program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
	//glUniform3f(glGetUniformLocation(shader.Program, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	glBindTexture(GL_TEXTURE_2D, shadowMapColorTexture);//已写入draw

	//glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), cubePos)));
	ourModel.Draw(shader,shadowMapColorTexture);//沿用高斯模糊完了的结果 此时纹理信息为（E（depth），E（depth*depth））

}

//整个screen的渲染
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

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

