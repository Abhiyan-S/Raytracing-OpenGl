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

	Scene scene(shader);
	scene.UpdateCamera(camera);

	std::vector<Sphere> spheres = { 
									Sphere(glm::vec3(-0.5,0,2), 0.1f),
									Sphere(glm::vec3(0,0,10), 0.5),
									Sphere(glm::vec3(0.5,0,2), 0.1f)
								  };

	spheres[0].material.color = glm::vec3(1, 1, 0);
	spheres[1].material.color = glm::vec3(1, 0, 1);
	spheres[2].material.color = glm::vec3(0, 1, 1);

	GLuint ssbo_lights;
	std::vector<Light> lights = {
									Light(glm::vec3(0,5,5), glm::vec3(1,1,1), 1)
								   };

	scene.UpdateSpheres(spheres);
	scene.UpdateLights(lights);

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

		scene.UpdateCamera(camera);
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

	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}