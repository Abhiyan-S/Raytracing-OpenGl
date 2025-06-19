#pragma once
#include<glm/glm.hpp>

//MAKE SURE TO CHANGE THE STRUCTURES IN THE FRAGMENT SHADERS IF YOU ARE CHANGING THE STRUCTS BELOW

struct GPUMaterial {
	glm::vec3 col;
	float pad1;
	float roughness;
	int emits;
	float emissionStrength;
	float pad2;
	GPUMaterial(glm::vec3 col, float roughness, int emits, float emissionStrength) : col(col), roughness(roughness), emits(emits), emissionStrength(emissionStrength) {};
};

struct GPULight {
	glm::vec3 pos;
	float pad1;
	glm::vec3 col;
	float intensity;
	GPULight(glm::vec3 pos, glm::vec3 col, float intensity) : pos(pos), col(col), intensity(intensity) {};
};

struct GPUSphere {
	glm::vec3 pos;
	float r;
	GPUMaterial mat;
	GPUSphere(glm::vec3 pos, float r, GPUMaterial mat) : pos(pos), r(r), mat(mat) {};
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

struct GPUTriangle {
	glm::vec3 a;
	float pad1 = 0;
	glm::vec3 b;
	float pad2 = 0;
	glm::vec3 c;
	float pad3 = 0;
	float objectIndex;
	GPUTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, float objectIdx) : a(a), b(b), c(c), objectIndex(objectIdx), pad1(0), pad2(0), pad3(0) {};
};