#include<glad/glad.h>
#include "Objects.h"
#include "Shaders/Shader.h"
#include<glm/gtc/matrix_transform.hpp>
#include "GPUStructs.h"
#define CAMERA_BINDING 0
#define LIGHTS_BINDING 1
#define SPHERES_BINDING 2

Camera::Camera(glm::vec3 position, glm::vec3 dir, float horizontalFOV, float focalLength, float aspectRatio) { // FOV in degrees
	dir = glm::normalize(dir);
	horizontalFOV = glm::radians(horizontalFOV);

	right = -glm::cross(dir, glm::vec3(0, 1, 0));// without - sign, the axes are flipped
	up = glm::cross(dir, right);

	this->position = position; this->dir = dir; this->horizontalFOV = horizontalFOV; this->focalLength = focalLength; this->aspectRatio = aspectRatio;

	screenWidth = 2 * focalLength * glm::tan(horizontalFOV / 2);
	screenHeight = screenWidth / aspectRatio;
}

void Scene::InitCamera() {
	glGenBuffers(1, &ubo_cam);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_cam);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUCamera), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_BINDING, ubo_cam);

	GLuint blockIndex = glGetUniformBlockIndex(this->shader.ID, "Camera");
	glUniformBlockBinding(this->shader.ID, blockIndex, CAMERA_BINDING);
}

void Scene::InitSpheres() {
	glGenBuffers(1, &ssbo_spheres);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SPHERES_BINDING, ssbo_spheres);
}

void Scene::InitLights() {
	glGenBuffers(1, &ssbo_lights);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_lights);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHTS_BINDING, ssbo_lights);
}

void Scene::InitScene() {
	InitCamera();
	InitLights();
	InitSpheres();
}

Scene::Scene(Shader shader) : shader(shader) {
	InitScene();
}


void Scene::UpdateCamera(Camera &camera) {
	camera.right = -glm::cross(camera.dir, glm::vec3(0, 1, 0));// without - sign, the axes are flipped
	camera.up = glm::cross(camera.dir, camera.right);
	glBindBuffer(1, ubo_cam);
	GPUCamera camData;
	camData.pos = camera.position;
	camData.pad1 = 0;
	camData.dir = camera.dir;
	camData.pad2 = 0;
	camData.up = camera.up;
	camData.pad3 = 0;
	camData.right = camera.right;
	camData.pad4 = 0;
	camData.fov = camera.horizontalFOV;
	camData.focalLength = camera.focalLength;
	camData.aspectRatio = camera.aspectRatio;
	camData.screenWidth = camera.screenWidth;
	camData.screenHeight = camera.screenHeight;

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUCamera), &camData);
}

void Scene::UpdateLights(const std::vector<Light>& lights) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_lights);
	std::vector<GPULight> gpuLights;
	for (int i = 0; i < lights.size(); i++) {
		GPULight gpuLight = GPULight(lights[i].position, lights[i].color, lights[i].intensity);
		gpuLight.pad1 = 0;
		gpuLights.push_back(gpuLight);
	}
	glBufferData(GL_SHADER_STORAGE_BUFFER, gpuLights.size() * sizeof(GPULight), gpuLights.data(), GL_STATIC_DRAW);
}

void Scene::UpdateSpheres(const std::vector<Sphere>& spheres) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
	std::vector<GPUSphere> gpuSpheres;
	for (Sphere sphere : spheres) {
		gpuSpheres.push_back(GPUSphere(sphere.position, sphere.radius, GPUMaterial(sphere.material.color, sphere.material.roughness, sphere.material.emits? 1:0, sphere.material.emissionStrength)));
	}
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUSphere) * gpuSpheres.size(), gpuSpheres.data(), GL_STATIC_DRAW);
}

void Scene::Delete() {
	glDeleteBuffers(1, &ubo_cam);
	glDeleteBuffers(1, &ssbo_lights);
	glDeleteBuffers(1, &ssbo_spheres);
}

Material::Material(glm::vec3 color, float roughness){
	this->color = color;
	this->roughness = roughness;
	this->emits = false;
	this->emissionStrength = 0;
}