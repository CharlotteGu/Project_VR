#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> //function loader for opengl

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	GLuint ID; //Unsigned binary integer saving the identifier of the shader

	Shader(const char* vertexPath, const char* fragmentPath) //used when code in a file
	{
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str(); //from string to const char* (not very useful)
        const char* fShaderCode = fragmentCode.c_str();

        GLuint vertex = compileShader(vertexCode, GL_VERTEX_SHADER);
        GLuint fragment = compileShader(fragmentCode, GL_FRAGMENT_SHADER);
        ID = compileProgram(vertex, fragment);
	}

    Shader(std::string vShaderCode, std::string fShaderCode) //used when giving directly the string of the shader code
    {
        // Compile both shaders and join them into a program
        GLuint vertex = compileShader(vShaderCode, GL_VERTEX_SHADER);
        GLuint fragment = compileShader(fShaderCode, GL_FRAGMENT_SHADER);
        ID = compileProgram(vertex, fragment);
    }

    void use() { //installs the program object specified by program as part of current rendering state
        glUseProgram(ID);
    }

    // Set of methods to getUniformeLocation for various types 
    void setInteger(const GLchar *name, GLint value) { //Specify the value of a uniform var for the current program object
        glUniform1i(glGetUniformLocation(ID, name), value); //find the (int) location of the uniform, then assign it with the given value
    }

    void setFloat(const GLchar* name, GLfloat value) {
        glUniform1f(glGetUniformLocation(ID, name), value);
    }

    void setVector3f(const GLchar* name, GLfloat x, GLfloat y, GLfloat z) {
        glUniform3f(glGetUniformLocation(ID, name), x, y, z);
    }

    void setVector3f(const GLchar* name, const glm::vec3& value) {
        glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
    }

    void setMatrix4(const GLchar* name, const glm::mat4& matrix) {
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(matrix));
    }

private:
    GLuint compileShader(std::string shaderCode, GLenum shaderType)
    {
        GLuint shader = glCreateShader(shaderType); //create the object
        const char* code = shaderCode.c_str();  //cast the code
        glShaderSource(shader, 1, &code, NULL); //(what shader, 1 element in string array, array of pointers to shader code string, null-terminated)
        glCompileShader(shader); //compile source code in the shader

        GLchar infoLog[1024]; //buffer to store failure info, if needed
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //get the compile status code to check if worked
        if (!success)   
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog); //store the log info about the failure
            std::string t = "undetermined";
            if (shaderType == GL_VERTEX_SHADER) {
                t = "vertex shader";
            }
            else if (shaderType == GL_FRAGMENT_SHADER) {
                t = "fragment shader";
            }
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of the " << t << ": " << shaderType << infoLog << std::endl;
        }
        return shader;
    }

    GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader)
    {
        GLuint programID = glCreateProgram(); //create an empty object program

        glAttachShader(programID, vertexShader); //link the shaders to the program
        glAttachShader(programID, fragmentShader);
        glLinkProgram(programID); //links the program object + create executable of the shaders


        GLchar infoLog[1024];
        GLint success;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(programID, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR:  " << infoLog << std::endl;
        }
        return programID;
    }

};
#endif