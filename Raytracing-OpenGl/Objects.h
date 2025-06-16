#pragma once
#include<glm/glm.hpp>

class Camera {
public:
	glm::vec3 position;
	glm::vec3 dir;
	glm::vec3 up;
	glm::vec3 right;
	float horizontalFOV, focalLength, aspectRatio;

	float screenWidth, screenHeight;

	Camera(glm::vec3 position, glm::vec3 dir, float horizontalFOV, float focalLength, float aspectRatio) { // FOV in degrees
		dir = glm::normalize(dir);
		horizontalFOV = glm::radians(horizontalFOV);

		right = -glm::cross(dir, glm::vec3(0, 1, 0));// without - sign, the axes are flipped
		up = glm::cross(dir, right);

		this->position = position; this->dir = dir; this->horizontalFOV = horizontalFOV; this->focalLength = focalLength; this->aspectRatio = aspectRatio;

		screenWidth = 2 * focalLength * glm::tan(horizontalFOV / 2);
		screenHeight = screenWidth / aspectRatio;
	}
};

class Material {
public:
	glm::vec3 color;
	float roughness;
	Material(glm::vec3 color, float roughness) : color(color), roughness(roughness) {};
	Material() : color(glm::vec3(0.5, 0.5, 0.5)), roughness(1) {};
};

class Sphere {
public:
	glm::vec3 position;
	Material material;
	float radius;
	Sphere(glm::vec3 pos, float r) : position(pos), radius(r) {};
};



struct GPUSphere {
	glm::vec3 pos;
	float r;
	Material mat;
	GPUSphere(glm::vec3 pos, float r, Material mat) : pos(pos), r(r), mat(mat) {};
};

struct GPUCamera {
	glm::vec3 pos; //12
	float pad1; //16
	glm::vec3 dir; //28
	float pad2;// 32
	glm::vec3 up;
	float pad3;
	glm::vec3 right;
	float pad4;
	float fov;//36
	float focalLength;//40
	float aspectRatio;//44
	float screenWidth;//48
	float screenHeight;//52
};