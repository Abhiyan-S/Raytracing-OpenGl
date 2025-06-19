#pragma once
#include<glm/glm.hpp>
#include<vector>
#include "Shaders/Shader.h"
class Camera {
public:
	glm::vec3 position;
	glm::vec3 dir;
	glm::vec3 up;
	glm::vec3 right;
	float horizontalFOV, focalLength, aspectRatio;

	float screenWidth, screenHeight;

	Camera(glm::vec3 position, glm::vec3 dir, float horizontalFOV, float focalLength, float aspectRatio);
};

class Material {
public:
	glm::vec3 color;
	float roughness;
	bool emits;
	float emissionStrength;
	Material(glm::vec3 col, float roughness);
	Material() : color(glm::vec3(0.5, 0.5, 0.5)), roughness(1), emits(false), emissionStrength(0) {};
};

class Sphere {
public:
	glm::vec3 position;
	float radius;
	Material material;
	Sphere(glm::vec3 pos, float r) : position(pos), radius(r) {};
};

class Light {
public:
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
	Light(glm::vec3 position, glm::vec3 color, float intensity) : position(position), color(color), intensity(intensity) {};
};

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

class Scene {
public:
	Shader shader;
	GLuint ubo_cam;
	GLuint ssbo_lights;
	GLuint ssbo_spheres;

	Scene(Shader shader);
	void InitScene();
	void InitCamera();
	void InitLights();
	void InitSpheres();

	void UpdateSpheres(const std::vector<Sphere> &spheres);
	void UpdateCamera(Camera &camera);
	void UpdateLights(const std::vector<Light>& lights);

	void Delete();
};