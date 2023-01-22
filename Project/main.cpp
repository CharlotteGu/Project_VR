#include<iostream>	//to handle io
#include <glad/glad.h> //to not having to write everything each time
#include <GLFW/glfw3.h> //for the window and context
#include <glm/glm.hpp> //to handle matrices 
#include<glm/gtc/matrix_transform.hpp> //same
#include<glm/gtc/type_ptr.hpp> //same

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //to handle images (needed for textures)

#include <map> //needed for cubemap
#include "camera.h"	//import the parts of the code from another file
#include "shader.h"
#include "object.h"

const int width = 500;	//save window size
const int height = 500;

GLuint compileShader(std::string shaderCode, GLenum shaderType);	//headers of useful functions dvlp below
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);
void loadCubemapFace(const char * file, const GLenum& targetCube);


Camera camera(glm::vec3(0.0, 0.0, 0.0));	//initial camera position

int main(int argc, char* argv[])
{
	//Create the OpenGL context 
	if (!glfwInit()) {	//initialize the library
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);	//choose some parameters
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "Project", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);	//make window for current thread 
	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	glEnable(GL_DEPTH_TEST); //activate depth testing (from z-buffer) st only visible fragments are shown

	unsigned char* data;	//declare it here st we can easily reuse it (not important)
	

/*------------------ Sun handling ------------------ */
	char sunV[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/sunV.txt";	
	char sunF[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/sunF.txt";
	Shader shaderSun(sunV, sunF); //create an object shader with the code saved in the previous files

	GLuint textureSun;	//global state machine: create obj and get id, bind to say you'll use it, act on it, unbind to stop working with it
	glGenTextures(1, &textureSun); //1 text name to be generated (id), array containing the texture name
	glBindTexture(GL_TEXTURE_2D, textureSun); //bind the target TEXT_2D (because 2D image) to the text whose name is given

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //set texture parameters -> wraping around s coordinate ((x,y,z) // (s,r,t)) is mirrored_rep (repeated, but mirrored when the integer part of the coordinate is odd)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); //same for t coordinate
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //texture coordinates are resolution independent, they won't always match a pixel exactly. Here, if imaged scalled down, we sample the closest mipmap with linear interpolation.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //when scaling up, we do linear interpolation

	stbi_set_flip_vertically_on_load(true); //need to flip it because (0,0) on y-axis is not placed at expected corner 

	int imWidth, imHeight, imNrChannels;
	char texSun[128] = PATH_TO_TEXTURE "/sun.jpg";
	data = stbi_load(texSun, &imWidth, &imHeight, &imNrChannels, 0); //load the image (location, width, height, nb of color chanels, rgb format ?)
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data); //generate the text (target, mipmap level, text format, width, height, always 0, format & datatype of source im, actual image)
		glGenerateMipmap(GL_TEXTURE_2D); //create mipmap for current bound texture image
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}

	char pathSun[] = PATH_TO_OBJECTS "/sphere_smooth.obj";
	Object sun(pathSun);	//create an object instance based on the file information
	sun.makeObject(shaderSun);	//handle the buffer and array to send every information to the object shader

	//clean up texture
	stbi_image_free(data); //free the image
	stbi_set_flip_vertically_on_load(false); //reflit to be back to normal

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

/*------------------ Second Planet handling ------------------ */	

	Shader shaderPlanet2(planetV, planetF); //re-use the same vertex code, but we'll tune some values differently

	GLuint texturePlanet2;
	glGenTextures(1, &texturePlanet2);
	glBindTexture(GL_TEXTURE_2D, texturePlanet2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);

	char texPlanet2[128] = PATH_TO_TEXTURE "/moon.jpg";
	data = stbi_load(texPlanet2, &imWidth, &imHeight, &imNrChannels, 0);
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

	Object planet2(pathPlanet);
	planet2.makeObject(shaderPlanet2);

	//clean up texture
	stbi_image_free(data);
	stbi_set_flip_vertically_on_load(false);

/*------------------ Dead Planet handling ------------------ */
	char DeadplanetV[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/deadPlanetV.txt";	
	char DeadplanetF[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/deadPlanetF.txt";
	
	Shader shaderDeadPlanet(DeadplanetV, DeadplanetF); //We reuse the same shaders for all because only the object info will change, not the way we use them (light)
	char pathDeadPlanet[] = PATH_TO_OBJECTS "/sphere_extremely_coarse.obj"; 
	Object deadPlanet(pathDeadPlanet);
	deadPlanet.makeObject(shaderDeadPlanet);

	Shader shaderDeadPlanet2(DeadplanetV, DeadplanetF);
	char pathDeadPlanet2[] = PATH_TO_OBJECTS "/sphere_coarse.obj";
	Object deadPlanet2(pathDeadPlanet2);
	deadPlanet2.makeObject(shaderDeadPlanet2);

	Shader shaderDeadPlanet3(DeadplanetV, DeadplanetF);
	char pathDeadPlanet3[] = PATH_TO_OBJECTS "/sphere_smooth.obj";
	Object deadPlanet3(pathDeadPlanet3);
	deadPlanet3.makeObject(shaderDeadPlanet3);

/*------------------ Cubmap handling ------------------ */
	const std::string sourceVCubeMap = "#version 330 core\n"
		"in vec3 position; \n"
		//"in vec2 tex_coord; \n" not even useful for the cubemap
		//"in vec3 normal; \n"
		
		//only P and V are necessary
		"uniform mat4 V; \n"
		"uniform mat4 P; \n"

		"out vec3 texCoord_v; \n"

		" void main(){ \n"
		"texCoord_v = position;\n" //set the incoming local position vector as the outcoming texture coordinate for (interpolated) use in the fragment shader
		//remove translation info from view matrix to only keep rotation
		"mat4 V_no_t = mat4(mat3(V)) ;\n" //remove the translation section of transformation matrices by taking the upper-left 3x3 matrix of the 4x4 matrix.
		"vec4 pos = P * V_no_t * vec4(position, 1.0); \n"
		// the positions xyz are divided by w after the vertex shaderSun
		// the z component is equal to the depth value
		// we want a z always equal to 1.0 here, so we set z = w!
		// Remember: z=1.0 is the MAXIMUM depth value ;)
		"gl_Position = pos.xyww;\n" //otherwise we do not see the objects because they are behind the cube
		"\n" 
		"}\n";

	const std::string sourceFCubeMap = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"precision mediump float; \n"
		"uniform samplerCube cubemapSampler; \n" //specific type of sampler
		"in vec3 texCoord_v; \n" //cube centered at origin so sufficient textCoord for having right texture at right coordinate
		"void main() { \n"
		"FragColor = texture(cubemapSampler,texCoord_v); \n"
		"} \n";

	//char cubemapV[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/cubemapV.txt";	
	//char cubemapF[128] = "/Users/cha/Doc/Universite/Informatique/MA2/H502- Virtual reality/Projet/Project/shadersCode/cubemapF.txt";
	Shader cubeMapShader = Shader(sourceVCubeMap, sourceFCubeMap); //for some reason, trouble putting this one in an externalized file

	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	Object cubeMap(pathCube);
	cubeMap.makeObject(cubeMapShader);

	GLuint cubeMapTexture;
	glGenTextures(1, &cubeMapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //specific wrapping for the cubemap
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/interstellar/";
	std::string extension = ".png";

	std::map<std::string, GLenum> facesToLoad = { 
		{pathToCubeMap + "posx" + extension,GL_TEXTURE_CUBE_MAP_POSITIVE_X}, //use special texture target for each face
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


/*------------------ Frame function ------------------ */ //From practicals, just kept it
	double prev = 0;
	int deltaFrame = 0;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {	//update the count periodically
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
	glm::vec3 light_pos = glm::vec3(0.0, 0.0, -5.0); 

	//sun model
	glm::mat4 modelSun = glm::mat4(1.0);
	modelSun = glm::translate(modelSun, light_pos);//same position as light, st it is the source
	modelSun = glm::scale(modelSun, glm::vec3(0.9, 0.9, 0.9)); 
	glm::mat4 inverseModelSun = glm::transpose(glm::inverse(modelSun)); //necessary for the normal

	//planet model
	glm::mat4 modelPlanet = glm::mat4(1.0);
	modelPlanet = glm::translate(modelPlanet, glm::vec3(0.3, -0.3, -2.5));
	modelPlanet = glm::scale(modelPlanet, glm::vec3(0.4, 0.4, 0.4));
	glm::mat4 inverseModelPlanet = glm::transpose(glm::inverse(modelPlanet));

	glm::mat4 rotationPlanet = glm::mat4(1.0); //prepare another matrix for the rotation on itself
	//rotationPlanet = glm::rotate(rotationPlanet, glm::radians((float)(1.0)),glm::vec3(0.0,1.0,0.0)); only done in the rendering ok

	//planet2 model
	glm::mat4 modelPlanet2 = glm::mat4(1.0);
	modelPlanet2 = glm::translate(modelPlanet2, glm::vec3(0.0, -2.0, -2.0));
	modelPlanet2 = glm::scale(modelPlanet2, glm::vec3(0.5, 0.5, 0.5));
	glm::mat4 inverseModelPlanet2 = glm::transpose(glm::inverse(modelPlanet2));

	glm::mat4 rotationPlanet2 = glm::mat4(1.0);
	//rotationPlanet2 = glm::rotate(rotationPlanet2, glm::radians((float)(1.5)),glm::vec3(-1.0,0.0,0.0));
	

	//dead planet model
	glm::mat4 modelDeadPlanet = glm::mat4(1.0);
	modelDeadPlanet = glm::translate(modelDeadPlanet, glm::vec3(3.0, 4.5, -8.0));
	modelDeadPlanet = glm::scale(modelDeadPlanet, glm::vec3(0.6, 0.6, 0.6));
	glm::mat4 inverseModelDeadPlanet = glm::transpose(glm::inverse(modelDeadPlanet));

	//dead planet2 model
	glm::mat4 modelDeadPlanet2 = glm::mat4(1.0);
	modelDeadPlanet2 = glm::translate(modelDeadPlanet2, glm::vec3(0.0, 4.5, -8.0));
	modelDeadPlanet2 = glm::scale(modelDeadPlanet2, glm::vec3(0.6, 0.6, 0.6));
	glm::mat4 inverseModelDeadPlanet2 = glm::transpose(glm::inverse(modelDeadPlanet2));

	//dead planet3 model
	glm::mat4 modelDeadPlanet3 = glm::mat4(1.0);
	modelDeadPlanet3 = glm::translate(modelDeadPlanet3, glm::vec3(1.5, 4.5, -10.0));
	modelDeadPlanet3 = glm::scale(modelDeadPlanet3, glm::vec3(0.6, 0.6, 0.6));
	glm::mat4 inverseModelDeadPlanet3 = glm::transpose(glm::inverse(modelDeadPlanet3));


	//camera info
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();


/*------------------ Passing light values to shaders ------------------ */

	//light parameters
	float ambient = 0.2;
	float diffuse = 0.5;
	float specular = 0.8;
	glm::vec3 light_color = glm::vec3(0.8,0.5,0.0); //make the light orange-yellowish

	shaderPlanet.use();	//say we care about this shader
	shaderPlanet.setFloat("shininess", 32.0f); //specify shininess (to bound the specular reflection)
	shaderPlanet.setFloat("light.ambient_strength", ambient);
	shaderPlanet.setFloat("light.diffuse_strength", diffuse);
	shaderPlanet.setFloat("light.specular_strength", specular);
	shaderPlanet.setVector3f("light.light_pos", light_pos);
	shaderPlanet.setVector3f("u_light_color", light_color);

	shaderPlanet2.use();
	shaderPlanet2.setFloat("shininess", 15.0f); //play with shininess
	shaderPlanet2.setFloat("light.ambient_strength", ambient);
	shaderPlanet2.setFloat("light.diffuse_strength", diffuse);
	shaderPlanet2.setFloat("light.specular_strength", specular);
	shaderPlanet2.setVector3f("light.light_pos", light_pos);
	shaderPlanet2.setVector3f("u_light_color", light_color);

	shaderDeadPlanet.use();
	shaderDeadPlanet.setFloat("shininess", 20.0f);
	shaderDeadPlanet.setFloat("light.ambient_strength", 0.5); 	//higher than other st we better see reflection
	shaderDeadPlanet.setFloat("light.diffuse_strength", diffuse);
	shaderDeadPlanet.setFloat("light.specular_strength", specular);
	shaderDeadPlanet.setVector3f("light.light_pos", light_pos);
	shaderDeadPlanet.setVector3f("u_light_color", light_color);

	shaderDeadPlanet2.use();
	shaderDeadPlanet2.setFloat("shininess", 18.0f);
	shaderDeadPlanet2.setFloat("light.ambient_strength", 0.5); 	//higher than other st we better see reflection
	shaderDeadPlanet2.setFloat("light.diffuse_strength", diffuse);
	shaderDeadPlanet2.setFloat("light.specular_strength", specular);
	shaderDeadPlanet2.setVector3f("light.light_pos", light_pos);
	shaderDeadPlanet2.setVector3f("u_light_color", light_color);

	shaderDeadPlanet3.use();
	shaderDeadPlanet3.setFloat("shininess", 16.0f);
	shaderDeadPlanet3.setFloat("light.ambient_strength", 0.5); 	//higher than other st we better see reflection
	shaderDeadPlanet3.setFloat("light.diffuse_strength", diffuse);
	shaderDeadPlanet3.setFloat("light.specular_strength", specular);
	shaderDeadPlanet3.setVector3f("light.light_pos", light_pos);
	shaderDeadPlanet3.setVector3f("u_light_color", light_color);

	
/*------------------ Rendering loop ------------------ */
	glfwSwapInterval(1); //time before context update
	while (!glfwWindowShouldClose(window)) {
		//initial settings
		processInput(window);	//check specific keyboard presses
		view = camera.GetViewMatrix();	//get the new view (in case the camera has changed)
		glfwPollEvents(); //check for event trigger (with keyboard for ex)
		double now = glfwGetTime();	//needed for fps called below
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f); //color the entire screen to not keep any previous color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the buffers

		//SUN
			//give sun information to shaders
		shaderSun.use();
		shaderSun.setMatrix4("M", modelSun);	//send all model uniform
		shaderSun.setMatrix4("itM", inverseModelSun);
		shaderSun.setMatrix4("V", view);
		shaderSun.setMatrix4("P", perspective);
		shaderSun.setVector3f("u_view_pos", camera.Position);
		float var_light = std::max(std::sin(1.2*now)+0.5, 0.4); //use a sin function to make the sun pulse
		shaderSun.setFloat("var_light", var_light);
			//texture
		shaderSun.setInteger("u_texture", 0);
		glActiveTexture(GL_TEXTURE0);	//need to specify which texture was for this object
		glBindTexture(GL_TEXTURE_2D, textureSun);
			//show the result
		glDepthFunc(GL_LEQUAL); //specify depth function. Here, ok if the incoming depth value is <= to the stored depth value.
		sun.draw(); //ask for the object primitives rendering

		//PLANET
			//give planet information to shaders
		shaderPlanet.use();
		shaderPlanet.setMatrix4("V", view);
		shaderPlanet.setMatrix4("P", perspective);
		shaderPlanet.setVector3f("u_view_pos", camera.Position);
		shaderPlanet.setFloat("var_light", var_light);
			//texture
		shaderPlanet.setInteger("u_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texturePlanet);

		//glDepthFunc(GL_LEQUAL); only need to specify it once since we did not change it
			//rotation around sun
		modelPlanet = glm::translate(modelPlanet, glm::vec3(0.1,0.0,0.0)); //make the model translate in direction x (right)
		modelPlanet = glm::rotate(modelPlanet,glm::radians((float)(1.0)),glm::vec3(0.0,4.0,0.0)); //in the same matrix, add a rotation around y axis (up) => the combination gives the rotation around the sun

		rotationPlanet = glm::rotate(rotationPlanet,glm::radians((float)(0.5)),glm::vec3(0.0,4.0,0.0)); //another rotation matrix, still around y axis
		glm::mat4 finalPos = modelPlanet * rotationPlanet; //compute the final positin of the planet with both movements
		shaderPlanet.setMatrix4("M", finalPos);
		inverseModelPlanet = glm::transpose(glm::inverse(finalPos)); //compute the normal matrices after both movements st light is correctly applied
		shaderPlanet.setMatrix4("itM", inverseModelPlanet);
		planet.draw();

		//PLANET2
			//give planet information to shaders
		shaderPlanet2.use();
		shaderPlanet2.setMatrix4("V", view);
		shaderPlanet2.setMatrix4("P", perspective);
		shaderPlanet2.setVector3f("u_view_pos", camera.Position);
		shaderPlanet2.setFloat("var_light", var_light);
			//texture
		shaderPlanet2.setInteger("u_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texturePlanet2);

		//glDepthFunc(GL_LEQUAL); 
			//rotation around sun
		modelPlanet2 = glm::translate(modelPlanet2, glm::vec3(0.12,0.0,0.0));
		modelPlanet2 = glm::rotate(modelPlanet2,glm::radians((float)(0.9)),glm::vec3(0.0,1.0,1.0));

		rotationPlanet2 = glm::rotate(rotationPlanet2,glm::radians((float)(0.7)),glm::vec3(0.0,4.0,0.0));
		glm::mat4 finalPos2 = modelPlanet2 * rotationPlanet2;
		shaderPlanet2.setMatrix4("M", finalPos2);
		inverseModelPlanet2 = glm::transpose(glm::inverse(finalPos2));
		shaderPlanet2.setMatrix4("itM", inverseModelPlanet2);
		planet2.draw();

		//DEAD PLANET
			//give information to shaders
		shaderDeadPlanet.use();
		shaderDeadPlanet.setMatrix4("M", modelDeadPlanet); 
		shaderDeadPlanet.setMatrix4("itM", inverseModelDeadPlanet);
		shaderDeadPlanet.setMatrix4("V", view);
		shaderDeadPlanet.setMatrix4("P", perspective);
		shaderDeadPlanet.setVector3f("u_view_pos", camera.Position);
		shaderDeadPlanet.setFloat("var_light", var_light);
			//show the result
		//glDepthFunc(GL_LEQUAL); 
		deadPlanet.draw();

		//DEAD PLANET2
			//give information to shaders
		shaderDeadPlanet2.use();
		shaderDeadPlanet2.setMatrix4("M", modelDeadPlanet2); 
		shaderDeadPlanet2.setMatrix4("itM", inverseModelDeadPlanet2);
		shaderDeadPlanet2.setMatrix4("V", view);
		shaderDeadPlanet2.setMatrix4("P", perspective);
		shaderDeadPlanet2.setVector3f("u_view_pos", camera.Position);
		shaderDeadPlanet2.setFloat("var_light", var_light);
			//show the result
		//glDepthFunc(GL_LEQUAL); 
		deadPlanet2.draw();

		//DEAD PLANET3
			//give information to shaders
		shaderDeadPlanet3.use();
		shaderDeadPlanet3.setMatrix4("M", modelDeadPlanet3); 
		shaderDeadPlanet3.setMatrix4("itM", inverseModelDeadPlanet3);
		shaderDeadPlanet3.setMatrix4("V", view);
		shaderDeadPlanet3.setMatrix4("P", perspective);
		shaderDeadPlanet3.setVector3f("u_view_pos", camera.Position);
		shaderDeadPlanet3.setFloat("var_light", var_light);
			//show the result
		//glDepthFunc(GL_LEQUAL); 
		deadPlanet3.draw();

		//CUBEMAP
		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		cubeMapShader.setInteger("cubemapTexture", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP,cubeMapTexture);
		cubeMap.draw();
		//glDepthFunc(GL_LESS); would be important if that was the first object drawn

		//final settings
		fps(now); //display the fps
		glfwSwapBuffers(window); //show the 2D buffer image on screen
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate(); //clean/delete eveyr glfw resources that was allocated

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