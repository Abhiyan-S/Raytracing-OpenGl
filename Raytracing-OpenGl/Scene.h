#pragma once

#include "Shaders/Shader.h"
#include <vector>
#include "Objects.h"
#include <glad/glad.h>
class Scene {
public:
	Shader raytracingShader;
	Shader accumShader;
	Shader displayShader;

	GLuint ubo_cam;
	GLuint ssbo_lights;
	GLuint ssbo_spheres;
	GLuint ssbo_objects;
	GLuint ssbo_triangles;

	std::vector<Sphere>* spheres;
	std::vector<Light>* lights;

	GLuint sceneFBO, sceneTex;
	GLuint accumFBO[2], accumTex[2];

	Scene(int width, int height, Shader raytracingShader, Shader accumShader, Shader displayShader);
	void InitScene(int width, int height);
	void InitCamera();
	void InitLights();
	void InitSpheres();
	void InitObjects();
	void InitFrameAccumulation(int width, int height);

	//All of these functions send data to the GPU
	//Avoid Calling these functions in program loop(exception UpdateCamera()) for now

	//TODO: This class now stores the references to objects and light vectors, no need to pass them to these functions
	void UpdateSpheres(const std::vector<Sphere>& spheres);
	void UpdateCamera(Camera& camera);
	void UpdateLights(const std::vector<Light>& lights);
	void UpdateObjects(const std::vector<Object>& objects);

	void Delete(std::vector<Sphere> &spheres);
};