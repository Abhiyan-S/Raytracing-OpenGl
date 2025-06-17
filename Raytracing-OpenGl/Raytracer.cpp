#include<SDL.h>
#include<glad/glad.h>
#include "Shaders/Shader.h"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include "Objects.h"
#include<vector>
#include<iostream>
#include<chrono>

bool running = true;

#define CAMERA_BINDING 0
#define LIGHTS_BINDING 1
#define SPHERES_BINDING 2

void CreateScreenQuad(GLuint *vao, GLuint *vbo) {
	float verts[] = {
		-1,1,
		 1,1,
		 1,-1,
		 1,-1,
		 -1,-1,
		 -1,1
	};
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void SendSpheresToGPU(GLuint *ssbo,const std::vector<Sphere> &spheres) {

	//TODO: split this function into initiliaztion and function to send it to gpu
	std::vector<GPUSphere> gpuSpheres;
	for (int i = 0; i < spheres.size(); i++) {
		gpuSpheres.push_back(GPUSphere(spheres[i].position, spheres[i].radius, spheres[i].material));
		
	}
	glGenBuffers(1, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUSphere) * gpuSpheres.size(),gpuSpheres.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SPHERES_BINDING, *ssbo);
}

void UpdateCamera(GLuint* ubo_cam, const Camera &camera) {
	glBindBuffer(GL_UNIFORM_BUFFER, *ubo_cam);
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

void SendLightsToGPU(GLuint *ssbo, const std::vector<Light> &lights) {
	glGenBuffers(1, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo);
	std::vector<GPULight> gpuLights;
	for (int i = 0; i < lights.size(); i++) {
		GPULight gpuLight = GPULight(lights[i].position, lights[i].color, lights[i].intensity);
		gpuLight.pad1 = 0;
		gpuLights.push_back(gpuLight);
	}
	glBufferData(GL_SHADER_STORAGE_BUFFER, gpuLights.size() * sizeof(GPULight), gpuLights.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHTS_BINDING, *ssbo);
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);

	int width = 800, height = 600;
	SDL_Window* window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
	glViewport(0, 0, width, height);

	SDL_Event event;

	GLuint vao_quad, vbo_quad;
	CreateScreenQuad(&vao_quad, &vbo_quad);

	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "OpenGL version: " << version << std::endl;

	Shader shader("Shaders/QuadVertexShader.vs", "Shaders/QuadFragShader.fs");
	int resLoc = glGetUniformLocation(shader.ID, "resolution");

	Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), 60, 10, width / (float)height);

	//TODO: Wrap the camera initialization and updating into two functions
	GLuint ubo_cam;
	glGenBuffers(1, &ubo_cam);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_cam);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUCamera), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, CAMERA_BINDING, ubo_cam);

	GLuint blockIndex = glGetUniformBlockIndex(shader.ID, "Camera");
	glUniformBlockBinding(shader.ID, blockIndex, CAMERA_BINDING);

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

	std::vector<Sphere> spheres = { 
									Sphere(glm::vec3(-0.5,0,2), 0.1f),
									Sphere(glm::vec3(0,0,10), 0.5),
									Sphere(glm::vec3(0.5,0,2), 0.1f)
								  };

	spheres[0].material.color = glm::vec3(1, 1, 0);
	spheres[1].material.color = glm::vec3(1, 0, 1);
	spheres[2].material.color = glm::vec3(0, 1, 1);
	GLuint ssbo_spheres;
	SendSpheresToGPU(&ssbo_spheres, spheres);

	GLuint ssbo_lights;
	std::vector<Light> lights = {
									Light(glm::vec3(0,5,5), glm::vec3(1,1,1), 1)
								   };
	SendLightsToGPU(&ssbo_lights, lights);

	//!!!DO NOT USE SENDSPHERES FUNCTIONS IN THE LOOP
	float time = 0;
	long double dt = 0;

	float fpsTimer = 0;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	float speed = 5;
	while (running) {
		auto start = std::chrono::high_resolution_clock::now();
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;
		}
		if (keystate[SDL_SCANCODE_W]) camera.position += camera.dir * speed * (float)dt;
		if (keystate[SDL_SCANCODE_S]) camera.position -= camera.dir * speed * (float)dt;
		if (keystate[SDL_SCANCODE_A]) camera.position -= camera.right * speed * (float)dt;
		if (keystate[SDL_SCANCODE_D]) camera.position += camera.right * speed * (float)dt;
		if (keystate[SDL_SCANCODE_Q]) camera.position -= camera.up * speed * (float)dt;
		if (keystate[SDL_SCANCODE_E]) camera.position += camera.up * speed * (float)dt;

		UpdateCamera(&ubo_cam, camera);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Use();
		glBindVertexArray(vao_quad);
		glUniform2f(resLoc, width, height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		
		SDL_GL_SwapWindow(window);
		auto end = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000000.0;
		time += dt;
		if (time - fpsTimer > 3) {
			fpsTimer = time;
			std::cout << "FPS: " << 1 / dt << "\n";
		}
	}
	glDeleteBuffers(1, &ubo_cam);
	glDeleteBuffers(1, &ssbo_spheres);
	glDeleteBuffers(1, &ssbo_lights);
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}