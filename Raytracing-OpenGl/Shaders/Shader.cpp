#include "Shader.h"
#include<glad/glad.h>
#include<fstream>
#include<sstream>
#include<string>
#include<iostream>

using namespace std;
Shader::Shader(const char* vertexFilePath, const char* fragmentFilePath) {
	ifstream vertexShaderFile(vertexFilePath, ios::binary | ios::in);
	ifstream fragmentShaderFile(fragmentFilePath, ios::binary | ios::in);

	stringstream vStream, fStream;
	vStream << vertexShaderFile.rdbuf();
	fStream << fragmentShaderFile.rdbuf();

	string vertexShaderCode = vStream.str(), fragmentShaderCode = fStream.str();

	vertexShaderFile.close();
	fragmentShaderFile.close();

	const char* vCode_c = vertexShaderCode.c_str();
	const char* fCode_c = fragmentShaderCode.c_str();

	int success;
	char logInfo[512];

	unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vCode_c, NULL);
	glCompileShader(vs);

	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vs, 512, NULL, logInfo);
		std::cout << "Error compiling vertex shader \n" << logInfo << std::endl;
	}

	unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fCode_c, NULL);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fs, 512, NULL, logInfo);
		std::cout << "Error compiling fragment shader \n" << logInfo << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID, vs); glAttachShader(ID, fs);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, logInfo);
		std::cout << "Error linking shader program:\n" << logInfo << std::endl;
	}


	glDeleteShader(vs); glDeleteShader(fs);

}

void Shader::Use() {
	glUseProgram(ID);
}