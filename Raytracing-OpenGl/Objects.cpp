#include<glad/glad.h>
#include "Objects.h"
#include "Shaders/Shader.h"
#include<glm/gtc/matrix_transform.hpp>
#include "GPUStructs.h"

#define PI 3.141592653
#define SPHERE_RESOLUTION 10

Camera::Camera(glm::vec3 position, glm::vec3 dir, float horizontalFOV, float focalLength, float aspectRatio) { // FOV in degrees
	dir = glm::normalize(dir);
	horizontalFOV = glm::radians(horizontalFOV);

	right = -glm::cross(dir, glm::vec3(0, 1, 0));// without - sign, the axes are flipped
	up = glm::cross(dir, right);

	this->position = position; this->dir = dir; this->horizontalFOV = horizontalFOV; this->focalLength = focalLength; this->aspectRatio = aspectRatio;

	screenWidth = 2 * focalLength * glm::tan(horizontalFOV / 2);
	screenHeight = screenWidth / aspectRatio;
}

Sphere::Sphere(glm::vec3 pos, float r){
	this->position = pos;
	this->radius = r;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	ConstructSphere();

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void Sphere::Render() {
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6 * SPHERE_RESOLUTION * SPHERE_RESOLUTION);
}

glm::vec3 SphericalToCartesian(float r, float t, float p) {
	float x = r * sin(t) * cos(p);
	float y = r * cos(t);
	float z = r * sin(t) * sin(p);

	return glm::vec3(x, y, z);
}

void Sphere::ConstructSphere() {
	std::vector<float> data;
	float res = (float)SPHERE_RESOLUTION;
	for (int i = 0; i < res; i++) {
		float theta = (i / res) * PI;
		float theta1 = ((i + 1) / res) * PI;
		for (int j = 0; j < res; j++) {
			float phi = (j / res) * 2 * PI;
			float phi1 = ((j + 1) / res) * 2 * PI;

			glm::vec3 p0 = SphericalToCartesian(this->radius, theta, phi);
			glm::vec3 p1 = SphericalToCartesian(this->radius, theta1, phi);
			glm::vec3 p2 = SphericalToCartesian(this->radius, theta1, phi1);
			glm::vec3 p3 = SphericalToCartesian(this->radius, theta, phi1);

			
				
			data.insert(data.end(), {
					p0.x, p0.y, p0.z, material.color.r, material.color.g, material.color.b,
					p1.x, p1.y, p1.z, material.color.r, material.color.g, material.color.b,
					p2.x, p2.y, p2.z, material.color.r, material.color.g, material.color.b
				});

			data.insert(data.end(), {
					p2.x, p2.y, p2.z, material.color.r, material.color.g, material.color.b,
					p3.x, p3.y, p3.z, material.color.r, material.color.g, material.color.b,
					p0.x, p0.y, p0.z, material.color.r, material.color.g, material.color.b
				});


		}
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_STATIC_DRAW);
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


