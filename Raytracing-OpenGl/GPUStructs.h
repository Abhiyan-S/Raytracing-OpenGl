#pragma once
#include<glm/glm.hpp>

//!!!!MAKE SURE TO CHANGE THE STRUCTURES IN THE FRAGMENT SHADERS IF YOU ARE CHANGING THE STRUCTS BELOW!!!!

struct GPUMaterial { //32 bytes
	glm::vec3 col;
	float roughness;
	int emits;
	float emissionStrength;
	float pad1 = 0;
	float pad2 = 0;
	GPUMaterial(glm::vec3 col, float roughness, bool emits, float emissionStrength) : col(col), roughness(roughness), emits(emits? 1:0), emissionStrength(emissionStrength) {};
};

struct GPULight {
	glm::vec3 pos;
	float pad1 = 0;
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
	glm::vec3 up;//44
	float pad3;//48
	glm::vec3 right;//60
	float fov;//64
	float focalLength;//68
	float aspectRatio;//72
	float screenWidth;//76
	float screenHeight;//80
};

struct GPUTriangle { // 64 bytes
	glm::vec3 a;
	float pad1 = 0;
	glm::vec3 b;
	float pad2 = 0;
	glm::vec3 c;
	float pad3 = 0;
	glm::vec3 normal;
	int objectIndex;
	GPUTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 normal, int objectIdx) : a(a), b(b), c(c), normal(normal), objectIndex(objectIdx) {};
};

struct GPUObject { // 64 bytes
	glm::vec3 pos;
	float scale;
	int trigStartIdx;
	int trigCount;
	GPUMaterial mat;
	float pad1 = 0;
	float pad2 = 0;
	GPUObject(glm::vec3 pos, float scale, int trigStartIdx, int trigCount, GPUMaterial mat) : pos(pos), scale(scale), trigStartIdx(trigStartIdx), trigCount(trigCount), mat(mat) {};
};