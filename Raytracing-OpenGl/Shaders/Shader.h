#pragma once

class Shader {
public:
	unsigned int ID;
	Shader(const char* vertexFilePath, const char* fragmentFilePath);
	void Use();
};