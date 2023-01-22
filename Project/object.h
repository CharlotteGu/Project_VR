#ifndef OBJECT_H
#define OBJECT_H

#include<iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>


struct Vertex { //every vertex is composed of those three elements
	glm::vec3 Position; //3D coordinate of vertex in space
	glm::vec2 Texture;	//2D associated texture coordinate for that point
	glm::vec3 Normal;	//3D vector of the normal (for ex, to see where the exterior is)
};


class Object
{
public:
	std::vector<glm::vec3> positions; //to store every information
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;
	std::vector<Vertex> vertices; //initialize a vector of vertices (which will use info saved in previous vector)

	int numVertices;

	GLuint VBO, VAO; //the buffers
	//glm::mat4 model = glm::mat4(1.0); //the object model but unfortunately, I rather created a new one in main each time...

	Object(const char* path) {
		// Get the object information from the file
		std::ifstream infile(path);
		if (!infile){
			std::cerr << "File could not be opened!\n"; // Report error
			std::cerr << "Error code: " << strerror(errno); // Get some info as to why
		}
		std::string line;
		while (std::getline(infile, line)) //read line by line and handle it accordingly to the type (vertex, normal, texture or face)
		{
			std::istringstream iss(line);
			std::string indice;
			iss >> indice;
			if (indice == "v") {	
				float x, y, z;
				iss >> x >> y >> z;
				positions.push_back(glm::vec3(x, y, z));

			}
			else if (indice == "vn") {
				float x, y, z;
				iss >> x >> y >> z;
				normals.push_back(glm::vec3(x, y, z));
			}
			else if (indice == "vt") {
				float u, v;
				iss >> u >> v;
				textures.push_back(glm::vec2(u, v));
			}
			else if (indice == "f") { 	//for the faces, we combine three vertices, each with its position, normal and texture
				std::string f1, f2, f3;
				iss >> f1 >> f2 >> f3;

				std::string p, t, n;

				//for face 1
				Vertex v1;

				p = f1.substr(0, f1.find("/"));
				f1.erase(0, f1.find("/") + 1);
				t = f1.substr(0, f1.find("/"));
				f1.erase(0, f1.find("/") + 1);
				n = f1.substr(0, f1.find("/"));

				v1.Position = positions.at(std::stof(p) - 1);
				v1.Normal = normals.at(std::stof(n) - 1);
				v1.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v1);

				//for face 12
				Vertex v2;

				p = f2.substr(0, f2.find("/"));
				f2.erase(0, f2.find("/") + 1);
				t = f2.substr(0, f2.find("/"));
				f2.erase(0, f2.find("/") + 1);
				n = f2.substr(0, f2.find("/"));

				v2.Position = positions.at(std::stof(p) - 1);
				v2.Normal = normals.at(std::stof(n) - 1);
				v2.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v2);

				//for face 3
				Vertex v3;

				p = f3.substr(0, f3.find("/"));
				f3.erase(0, f3.find("/") + 1);
				t = f3.substr(0, f3.find("/"));
				f3.erase(0, f3.find("/") + 1);
				n = f3.substr(0, f3.find("/"));

				v3.Position = positions.at(std::stof(p) - 1);
				v3.Normal = normals.at(std::stof(n) - 1);
				v3.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v3);
			}
		}
		std::cout << "Load model with " << vertices.size() << " vertices" << std::endl;
		infile.close();		//do not forget to close the file 
		numVertices = vertices.size();	//keep track of the number of vertices we have
	}

	void makeObject(Shader shader, bool texture = true, bool normal = true) {
		// Put the object information (info of every vertex) in the buffer (data) for the shader to read
		float* data = new float[8 * numVertices];
		for (int i = 0; i < numVertices; i++) {
			Vertex v = vertices.at(i);
			data[i * 8] = v.Position.x;
			data[i * 8 + 1] = v.Position.y;
			data[i * 8 + 2] = v.Position.z;

			data[i * 8 + 3] = v.Texture.x;
			data[i * 8 + 4] = v.Texture.y;

			data[i * 8 + 5] = v.Normal.x;
			data[i * 8 + 6] = v.Normal.y;
			data[i * 8 + 7] = v.Normal.z;
		}

		glGenVertexArrays(1, &VAO); //create an array of 1 array (VA0)
		glGenBuffers(1, &VBO); //create one buffer (VBO)

		//define VBO and VAO as active buffer and active vertex array
		glBindVertexArray(VAO);	//say that we work with that vertex array 
		glBindBuffer(GL_ARRAY_BUFFER, VBO); //say that we work with that buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, data, GL_STATIC_DRAW); //put data into buffer

		auto att_pos = glGetAttribLocation(shader.ID, "position"); //Explain how to interpret the buffer; position is the name in the vShader !
		glEnableVertexAttribArray(att_pos);	
		glVertexAttribPointer(att_pos, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)0); //(attr, #elements to read, type, if normalized, how many bytes we skip, byte offset of first component)

		
		if (texture) {	//if we asked for texture, we also create an attribute for 
			auto att_tex = glGetAttribLocation(shader.ID, "tex_coord");
			glEnableVertexAttribArray(att_tex);
			glVertexAttribPointer(att_tex, 2, GL_FLOAT, false, 8 * sizeof(float), (void*)(3 * sizeof(float))); //NB: here only two coordinates to get and we have an offset
		}
		
		if (normal){ //same for normal
			auto att_norm = glGetAttribLocation(shader.ID, "normal");
			glEnableVertexAttribArray(att_norm);
			glVertexAttribPointer(att_norm, 3, GL_FLOAT, false, 8*sizeof(float), (void*)((5)*sizeof(float)));
		}
		
		/*auto att_col = glGetAttribLocation(shader.ID, "normal"); //only repeats the above...
		glEnableVertexAttribArray(att_col);
		glVertexAttribPointer(att_col, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)(5 * sizeof(float)));*/
		
		//desactive the buffer (and the array)
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		delete[] data; //empty the data because we do not need to keep them 
	}

	void draw() {
		glBindVertexArray(this->VAO); //get the right vertex array
		glDrawArrays(GL_TRIANGLES, 0, numVertices); //render primites
	}
};
#endif