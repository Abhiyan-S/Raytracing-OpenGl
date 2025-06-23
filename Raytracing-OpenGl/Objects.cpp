#include<glad/glad.h>
#include "Objects.h"
#include "Shaders/Shader.h"
#include<glm/gtc/matrix_transform.hpp>
#include "GPUStructs.h"

Camera::Camera(glm::vec3 position, glm::vec3 dir, float horizontalFOV, float focalLength, float aspectRatio) { // FOV in degrees
	dir = glm::normalize(dir);
	horizontalFOV = glm::radians(horizontalFOV);

	right = -glm::cross(dir, glm::vec3(0, 1, 0));// without - sign, the axes are flipped
	up = glm::cross(dir, right);

	this->position = position; this->dir = dir; this->horizontalFOV = horizontalFOV; this->focalLength = focalLength; this->aspectRatio = aspectRatio;

	screenWidth = 2 * focalLength * glm::tan(horizontalFOV / 2);
	screenHeight = screenWidth / aspectRatio;
}


Material::Material(glm::vec3 color, float roughness){
	this->color = color;
	this->roughness = roughness;
	this->emits = false;
	this->emissionStrength = 0;
}

Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 normal) {
	this->a = a;
	this->b = b;
	this->c = c;
	this->normal = normal;
}

Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
	this->a = a;
	this->b = b;
	this->c = c;
	this->normal = CalculateNormal();
}

glm::vec3 Triangle::CalculateNormal() {
	return glm::normalize(glm::cross(b-a, c-a));
}


