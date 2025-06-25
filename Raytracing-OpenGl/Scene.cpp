#include "Scene.h"
#include "glad/glad.h"
#include "GPUStructs.h"


#define CAMERA_BINDING 0
#define LIGHTS_BINDING 1
#define SPHERES_BINDING 2
#define OBJECTS_BINDING 3
#define TRIANGLES_BINDING 4

void Scene::InitCamera() {
	glGenBuffers(1, &ubo_cam);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_cam);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUCamera), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_BINDING, ubo_cam);

	GLuint blockIndex = glGetUniformBlockIndex(this->raytracingShader.ID, "Camera");
	glUniformBlockBinding(this->raytracingShader.ID, blockIndex, CAMERA_BINDING);
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

void Scene::InitObjects() {
	glGenBuffers(1, &ssbo_objects);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_objects);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, OBJECTS_BINDING, ssbo_objects);

	glGenBuffers(1, &ssbo_triangles);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TRIANGLES_BINDING, ssbo_triangles);
}


void Scene::InitFrameAccumulation(int width, int height) {
	glGenFramebuffers(1, &sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

	glGenTextures(1, &sceneTex);
	glBindTexture(GL_TEXTURE_2D, sceneTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTex, 0);


	glGenFramebuffers(2, accumFBO);
	glGenTextures(2, accumTex);

	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, accumFBO[i]);
		glBindTexture(GL_TEXTURE_2D, accumTex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTex[i], 0);
	}
}

void Scene::InitScene(int width, int height) {
	InitCamera();
	InitLights();
	InitSpheres();
	InitObjects();
	InitFrameAccumulation(width, height);
}

Scene::Scene(int width, int height, Shader raytracingShader, Shader accumShader, Shader displayShader) : raytracingShader(raytracingShader), accumShader(accumShader), displayShader(displayShader) {
	spheres = nullptr;
	lights = nullptr;
	InitScene(width, height);
}


void Scene::UpdateCamera(Camera& camera) {
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
	camData.fov = camera.horizontalFOV;
	camData.focalLength = camera.focalLength;
	camData.aspectRatio = camera.aspectRatio;
	camData.screenWidth = camera.screenWidth;
	camData.screenHeight = camera.screenHeight;

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUCamera), &camData);
}

//Avoid Calling these update functions in program loop(exception UpdateCamera()) for now
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
	std::vector<GPUSphere> gpuSpheres;
	for (const Sphere& sphere : spheres) {
		gpuSpheres.push_back(GPUSphere(sphere.position, sphere.radius, GPUMaterial(sphere.material.color, sphere.material.roughness, sphere.material.emits, sphere.material.emissionStrength)));
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUSphere) * gpuSpheres.size(), gpuSpheres.data(), GL_STATIC_DRAW);
}

void Scene::Delete(std::vector<Sphere> &spheres) {
	glDeleteBuffers(1, &ubo_cam);
	glDeleteBuffers(1, &ssbo_lights);
	glDeleteBuffers(1, &ssbo_spheres);

	for (Sphere& sphere : spheres) {
		glDeleteBuffers(1, &sphere.VBO);
		glDeleteVertexArrays(1, &sphere.VAO);
	}
}

void Scene::UpdateObjects(const std::vector<Object>& objects) {
	std::vector<GPUObject> gpuObjects;
	std::vector<GPUTriangle> gpuTriangles;

	int trigStartIdx = 0;
	int netTriangles = 0;
	for (int o = 0; o < objects.size(); o++) {
		for (const Triangle& trig : objects[o].triangles) {
			gpuTriangles.push_back(GPUTriangle((trig.a * objects[o].scale) + objects[o].position, (trig.b * objects[o].scale) + objects[o].position, (trig.c * objects[o].scale) + objects[o].position, trig.normal, o));
			netTriangles++;
		}
		int t = objects[o].triangles.size();
		gpuObjects.push_back(GPUObject(objects[o].position, objects[o].scale, trigStartIdx, t, GPUMaterial(objects[o].material.color, objects[o].material.roughness, objects[o].material.emits, objects[o].material.emissionStrength)));
		trigStartIdx += t;
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_objects);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUObject) * gpuObjects.size(), gpuObjects.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUTriangle) * netTriangles, gpuTriangles.data(), GL_STATIC_DRAW);
}