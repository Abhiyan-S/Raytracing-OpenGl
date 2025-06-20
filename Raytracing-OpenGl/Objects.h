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

class Triangle {
public:
	glm::vec3 a, b, c, normal;
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 normal);
	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3);
	glm::vec3 CalculateNormal();
};

class Object {
public:
	glm::vec3 position;
	float scale;
	Material material;
	std::vector<Triangle> triangles;

	Object(glm::vec3 pos, float s) : position(pos), scale(s) {};
	void AddTriangle(Triangle triangle) {
		triangles.push_back(triangle);
	}
};

class Scene {
public:
	Shader shader;
	GLuint ubo_cam;
	GLuint ssbo_lights;
	GLuint ssbo_spheres;
	GLuint ssbo_objects;
	GLuint ssbo_triangles;

	Scene(Shader shader);
	void InitScene();
	void InitCamera();
	void InitLights();
	void InitSpheres();

	//All of these functions send data to the GPU
	void UpdateSpheres(const std::vector<Sphere> &spheres);
	void UpdateCamera(Camera &camera);
	void UpdateLights(const std::vector<Light>& lights);
	void UpdateObjects(const std::vector<Object>& objects);

	void Delete();
};