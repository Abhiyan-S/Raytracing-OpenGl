#include<SDL.h>
#include<glad/glad.h>
#include "Shaders/Shader.h"
#include<glm/glm.hpp>
#include "Objects.h"
#include<vector>
#include<iostream>
bool running = true;

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
		gpuSpheres.push_back(GPUSphere(spheres[i].position, spheres[i].radius));
	}
	glGenBuffers(1, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUSphere) * gpuSpheres.size(),gpuSpheres.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, *ssbo);
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

	Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), 60, 5, width / (float)height);

	//TODO: Wrap the camera initialization and updating into two functions
	GLuint ubo_cam;
	glGenBuffers(1, &ubo_cam);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_cam);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUCamera), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_cam);

	GLuint blockIndex = glGetUniformBlockIndex(shader.ID, "Camera");
	glUniformBlockBinding(shader.ID, blockIndex, 1);

	GPUCamera camData;
	camData.pos = camera.position;
	camData.pad1 = 0; 
	camData.dir = camera.dir;
	camData.pad2 = 0;
	camData.fov = camera.horizontalFOV;
	camData.focalLength = camera.focalLength;
	camData.aspectRatio = camera.aspectRatio;
	camData.screenWidth = camera.screenWidth;
	camData.screenHeight = camera.screenHeight;

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GPUCamera), &camData);

	std::vector<Sphere> spheres = { 
									Sphere(glm::vec3(-0.5,0,2), 0.1f),
									Sphere(glm::vec3(0,0.5,2), 0.2f),
									Sphere(glm::vec3(0.5,0,2), 0.1f)
								  };
	GLuint ssbo_spheres;
	SendSpheresToGPU(&ssbo_spheres, spheres);

	//!!!DO NOT USE SENDSPHERES FUNCTIONS IN THE LOOP
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;
		}
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Use();
		glBindVertexArray(vao_quad);
		glUniform2f(resLoc, width, height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		
		SDL_GL_SwapWindow(window);
	}
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}