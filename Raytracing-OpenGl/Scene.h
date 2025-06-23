#pragma once

#include "Shaders/Shader.h"
#include <vector>
#include "Objects.h"
#include <glad/glad.h>
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
	void InitObjects();

	//All of these functions send data to the GPU
	//Avoid Calling these functions in program loop(exception UpdateCamera()) for now
	void UpdateSpheres(const std::vector<Sphere>& spheres);
	void UpdateCamera(Camera& camera);
	void UpdateLights(const std::vector<Light>& lights);
	void UpdateObjects(const std::vector<Object>& objects);

	void Delete();
};