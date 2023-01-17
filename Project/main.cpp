#include<iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <map>
#include "camera.h"
#include "shader.h"
#include "object.h"

const int width = 500;
const int height = 500;

GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);
void loadCubemapFace(const char * file, const GLenum& targetCube);


Camera camera(glm::vec3(0.0, 0.0, 0.0));

int main(int argc, char* argv[])
{
	//Create the OpenGL context 
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "Project", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);
	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	glEnable(GL_DEPTH_TEST);

	unsigned char* data;
	

/*------------------ Sun handling ------------------ */
	char sunV[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/sunV.txt";	
	char sunF[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/sunF.txt";
	Shader shaderSun(sunV, sunF);

	GLuint textureSun;
	glGenTextures(1, &textureSun);
	glBindTexture(GL_TEXTURE_2D, textureSun);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);

	int imWidth, imHeight, imNrChannels;
	char texSun[128] = PATH_TO_TEXTURE "/sun.jpg";
	data = stbi_load(texSun, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}

	char pathSun[] = PATH_TO_OBJECTS "/sphere_smooth.obj";
	Object sun(pathSun);
	sun.makeObject(shaderSun);

	//clean up texture
	stbi_image_free(data);
	stbi_set_flip_vertically_on_load(false);

/*------------------ Planet handling ------------------ */
	char planetV[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/planetV.txt";	
	char planetF[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/planetF.txt";
	Shader shaderPlanet(planetV, planetF);

	GLuint texturePlanet;
	glGenTextures(1, &texturePlanet);
	glBindTexture(GL_TEXTURE_2D, texturePlanet);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);

	char texPlanet[128] = PATH_TO_TEXTURE "/planet.jpg";
	data = stbi_load(texPlanet, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}

	char pathPlanet[] = PATH_TO_OBJECTS "/sphere_smooth.obj";
	Object planet(pathPlanet);
	planet.makeObject(shaderPlanet);

	//clean up texture
	stbi_image_free(data);
	stbi_set_flip_vertically_on_load(false);

/*------------------ Dead Planet handling ------------------ */
	char DeadplanetV[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/deadPlanetV.txt";	
	char DeadplanetF[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/deadPlanetF.txt";
	Shader shaderDeadPlanet(DeadplanetV, DeadplanetF);

	char pathDeadPlanet[] = PATH_TO_OBJECTS "/sphere_extremely_coarse.obj"; //todo: play with smoothness
	Object deadPlanet(pathDeadPlanet);
	deadPlanet.makeObject(shaderDeadPlanet);

	/*char pathDeadPlanet2[] = PATH_TO_OBJECTS "/sphere_coarse.obj";
	Object deadPlanet2(pathDeadPlanet2);
	deadPlanet2.makeObject(shaderDeadPlanet2);*/

/*------------------ Cubmap handling ------------------ */
	const std::string sourceVCubeMap = "#version 330 core\n"
		"in vec3 position; \n"
		"in vec2 tex_coord; \n"
		"in vec3 normal; \n"
		
		//only P and V are necessary
		"uniform mat4 V; \n"
		"uniform mat4 P; \n"

		"out vec3 texCoord_v; \n"

		" void main(){ \n"
		"texCoord_v = position;\n"
		//remove translation info from view matrix to only keep rotation
		"mat4 V_no_rot = mat4(mat3(V)) ;\n"
		"vec4 pos = P * V_no_rot * vec4(position, 1.0); \n"
		// the positions xyz are divided by w after the vertex shaderSun
		// the z component is equal to the depth value
		// we want a z always equal to 1.0 here, so we set z = w!
		// Remember: z=1.0 is the MAXIMUM depth value ;)
		"gl_Position = pos.xyww;\n"
		"\n" 
		"}\n";

	const std::string sourceFCubeMap = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"precision mediump float; \n"
		"uniform samplerCube cubemapSampler; \n"
		"in vec3 texCoord_v; \n"
		"void main() { \n"
		"FragColor = texture(cubemapSampler,texCoord_v); \n"
		"} \n";

	//char cubemapV[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/cubemapV.txt";	
	//char cubemapF[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/cubemapF.txt";
	Shader cubeMapShader = Shader(sourceVCubeMap, sourceFCubeMap);

	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	Object cubeMap(pathCube);
	cubeMap.makeObject(cubeMapShader);

	GLuint cubeMapTexture;
	glGenTextures(1, &cubeMapTexture);
	//glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//stbi_set_flip_vertically_on_load(true);

	std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/interstellar/";
	std::string extension = ".png";

	std::map<std::string, GLenum> facesToLoad = { 
		{pathToCubeMap + "posx" + extension,GL_TEXTURE_CUBE_MAP_POSITIVE_X},
		{pathToCubeMap + "posy" + extension,GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
		{pathToCubeMap + "posz" + extension,GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
		{pathToCubeMap + "negx" + extension,GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
		{pathToCubeMap + "negy" + extension,GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
		{pathToCubeMap + "negz" + extension,GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
	};
	//load the six faces
	for (std::pair<std::string, GLenum> pair : facesToLoad) {
		loadCubemapFace(pair.first.c_str(), pair.second);
	}


/*------------------ Frame function ------------------ */
	double prev = 0;
	int deltaFrame = 0;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {
			prev = now;
			const double fpsCount = (double)deltaFrame / deltaTime;
			deltaFrame = 0;
			std::cout << "\r FPS: " << fpsCount;
			std::cout.flush();
		}
	};

/*------------------ Camera elements + light + potential uniforme ------------------ */

	//order of positive coordinate (based on camera orientation): x=right, y=upward, z=backward

	//light source
	glm::vec3 light_pos = glm::vec3(0.0, 0.0, -2.0); //same position as sun, such that it is the light source

	//sun model
	glm::mat4 modelSun = glm::mat4(1.0);
	modelSun = glm::translate(modelSun, glm::vec3(0.0, 0.0, -5.0));
	modelSun = glm::scale(modelSun, glm::vec3(0.9, 0.9, 0.9));
	glm::mat4 inverseModelSun = glm::transpose(glm::inverse(modelSun));

	//planet model
	glm::mat4 modelPlanet = glm::mat4(1.0);
	modelPlanet = glm::translate(modelPlanet, glm::vec3(0.3, -0.3, -2.0));
	modelPlanet = glm::scale(modelPlanet, glm::vec3(0.4, 0.4, 0.4));
	glm::mat4 inverseModelPlanet = glm::transpose(glm::inverse(modelPlanet));

	glm::mat4 rotationPlanet = glm::mat4(1.0);
	rotationPlanet = glm::rotate(rotationPlanet, glm::radians((float)(1.0)),glm::vec3(0.0,1.0,0.0));
	

	//dead planet model
	glm::mat4 modelDeadPlanet = glm::mat4(1.0);
	modelDeadPlanet = glm::translate(modelDeadPlanet, glm::vec3(3.0, 1.5, -7.0));
	modelDeadPlanet = glm::scale(modelDeadPlanet, glm::vec3(0.5, 0.5, 0.5));
	glm::mat4 inverseModelDeadPlanet = glm::transpose(glm::inverse(modelDeadPlanet));


	//camera info
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	//light parameters
	float ambient = 0.1;
	float diffuse = 0.5;
	float specular = 0.8;

	//What is that ?
	//glm::vec3 materialColour = glm::vec3(0.5f,0.6,0.8);
	

/*------------------ Passing values to shaderSun ------------------ */

	shaderSun.use();
	shaderSun.setFloat("shininess", 32.0f);
	//shaderSun.setVector3f("materialColour", materialColour);
	shaderSun.setFloat("light.ambient_strength", ambient);
	shaderSun.setFloat("light.diffuse_strength", diffuse);
	shaderSun.setFloat("light.specular_strength", specular);
	shaderSun.setFloat("light.constant", 1.0);
	shaderSun.setFloat("light.linear", 0.14);
	shaderSun.setFloat("light.quadratic", 0.07);

	//shaderDeadPlanet.setFloat("refractionIndice", 1.52); //iced planet
	

/*------------------ Rendering loop ------------------ */
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window)) {
		//initial settings
		processInput(window);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//SUN
			//give sun information to shaders
		shaderSun.use();
		shaderSun.setMatrix4("M", modelSun);
		shaderSun.setMatrix4("itM", inverseModelSun);
		shaderSun.setMatrix4("V", view);
		shaderSun.setMatrix4("P", perspective);
		shaderSun.setVector3f("u_view_pos", camera.Position);
			//light
		auto deltaSun = light_pos + glm::vec3(0.0,0.0,2 * std::sin(now));
		shaderSun.setVector3f("light.light_pos", deltaSun);
			//texture
		shaderSun.setInteger("u_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureSun);
			//show the result
		glDepthFunc(GL_LEQUAL); //Show the object even if it's depth is equal to the depht of the object already present
		sun.draw();

		//PLANET
			//give planet information to shaders
		shaderPlanet.use();
		shaderPlanet.setMatrix4("M", modelPlanet); //try to do rotation in the rendering
		shaderPlanet.setMatrix4("itM", inverseModelPlanet);
		shaderPlanet.setMatrix4("V", view);
		shaderPlanet.setMatrix4("P", perspective);
		shaderPlanet.setMatrix4("R", rotationPlanet);
		shaderPlanet.setVector3f("u_view_pos", camera.Position);
			//light
		auto deltaPlanet = light_pos + glm::vec3(0.0,0.0,2 * std::sin(now));
		shaderPlanet.setVector3f("light.light_pos", deltaPlanet);
			//texture
		shaderPlanet.setInteger("u_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texturePlanet);
			//show the result
		glDepthFunc(GL_LEQUAL); //Show the object even if it's depth is equal to the depht of the object already present
		modelPlanet = glm::translate(modelPlanet, glm::vec3(0.1,0.0,0.0));
		modelPlanet = glm::rotate(modelPlanet,glm::radians((float)(1.0)),glm::vec3(0.0,4.0,0.0));
		rotationPlanet = glm::rotate(rotationPlanet,glm::radians((float)(0.5)),glm::vec3(0.0,4.0,0.0));
		planet.draw();

		//DEAD PLANET
			//give information to shaders
		shaderDeadPlanet.use();
		shaderDeadPlanet.setMatrix4("M", modelDeadPlanet); //try to do rotation in the rendering
		shaderDeadPlanet.setMatrix4("itM", inverseModelDeadPlanet);
		shaderDeadPlanet.setMatrix4("V", view);
		shaderDeadPlanet.setMatrix4("P", perspective);
		shaderDeadPlanet.setVector3f("u_view_pos", camera.Position);
		/*
			//light
		auto deltaPlanet = light_pos + glm::vec3(0.0,0.0,2 * std::sin(now));
		shaderPlanet.setVector3f("light.light_pos", deltaPlanet);*/
			//show the result
		glDepthFunc(GL_LEQUAL); 
		deadPlanet.draw();

		//CUBEMAP
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		cubeMapShader.setInteger("cubemapTexture", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP,cubeMapTexture);
		cubeMap.draw();
		glDepthFunc(GL_LESS);

		//final settings
		fps(now);
		glfwSwapBuffers(window);
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void loadCubemapFace(const char * path, const GLenum& targetFace)
{
	int imWidth, imHeight, imNrChannels;
	unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{
		glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}
	stbi_image_free(data);
}


void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(LEFT, 0.1);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(RIGHT, 0.1);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(FORWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(BACKWARD, 0.1);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(-1, 0.0, 1);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, 1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, -1.0, 1);
}


