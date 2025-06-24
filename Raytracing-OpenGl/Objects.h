#pragma once
#include<glm/glm.hpp>
#include<vector>
#include "Shaders/Shader.h"
#include<glad/glad.h>
class Camera {
public:
	float speed = 10;
	float sensitivity = 0.001;
	glm::vec2 resolution;
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
	GLuint VAO, VBO;
	Sphere(glm::vec3 pos, float r);
	void ConstructSphere();
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

