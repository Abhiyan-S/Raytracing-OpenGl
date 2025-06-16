#pragma once
#include<glm/glm.hpp>
class Camera {
public:
	glm::vec3 position;
	glm::vec3 dir;
	float horizontalFOV, focalLength, aspectRatio;

	float screenWidth, screenHeight;

	Camera(glm::vec3 position, glm::vec3 dir, float horizontalFOV, float focalLength, float aspectRatio) { // FOV in degrees
		dir = glm::normalize(dir);
		horizontalFOV = glm::radians(horizontalFOV);
		this->position = position; this->dir = dir; this->horizontalFOV = horizontalFOV; this->focalLength = focalLength; this->aspectRatio = aspectRatio;

		screenWidth = 2 * focalLength * glm::tan(horizontalFOV / 2);
		screenHeight = screenWidth / aspectRatio;
	}
};

class Sphere {
public:
	glm::vec3 position;
	float radius;
	Sphere(glm::vec3 pos, float r) : position(pos), radius(r) {};
};

struct GPUSphere {
	glm::vec3 pos;
	float r;
	GPUSphere(glm::vec3 pos, float r) : pos(pos), r(r) {};
};

struct GPUCamera {
	glm::vec3 pos; //12
	float pad1; //16
	glm::vec3 dir; //28
	float pad2;// 32
	float fov;//36
	float focalLength;//40
	float aspectRatio;//44
	float screenWidth;//48
	float screenHeight;//52
};