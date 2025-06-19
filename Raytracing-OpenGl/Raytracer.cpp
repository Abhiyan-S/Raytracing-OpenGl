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
int width = 800, height = 600;

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

	Scene scene(shader);
	scene.UpdateCamera(camera);

	std::vector<Sphere> spheres = { 
									Sphere(glm::vec3(0,-102,2), 100),
									Sphere(glm::vec3(0,1,0), 3),
									Sphere(glm::vec3(6,0,0), 2),
									Sphere(glm::vec3(10,0,0), 1)
								  };

	spheres[0].material.roughness = 1;
	spheres[0].material.color = glm::vec3(1, 1, 1);
	spheres[1].material.emits = true;
	spheres[1].material.emissionStrength = 4;
	spheres[1].material.color = glm::vec3(0.8, 0.1, 1);
	spheres[2].material.roughness = 0.1;
	spheres[2].material.color = glm::vec3(1, 1, 0);

	spheres[3].material.emits = true;
	spheres[3].material.emissionStrength = 4;
	spheres[3].material.color = glm::vec3(0.1, 1, 0.1);

	GLuint ssbo_lights;
	std::vector<Light> lights = {
									Light(glm::vec3(7,5,2), glm::vec3(1,1,1),1),
									Light(glm::vec3(2,5,8), glm::vec3(1,0.4,0.3),1)
								   };

	scene.UpdateSpheres(spheres);
	scene.UpdateLights(lights);

	float time = 0;
	float dt = 0;

	float fpsTimer = 0;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	GLuint seedLoc = glGetUniformLocation(shader.ID, "frameSeed");
	GLuint samplesLoc = glGetUniformLocation(shader.ID, "SAMPLES");
	int samples = 1;
	glUniform1i(samplesLoc, samples);
	float speed = 5;
	float sensitivity = 0.001;
	while (running) {
		auto start = std::chrono::high_resolution_clock::now();
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) running = false;
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
				if (event.key.keysym.sym == SDLK_RIGHT) { samples += 1; glUniform1i(samplesLoc, samples); std::cout << "Increase\n"; }
				if (event.key.keysym.sym == SDLK_LEFT) { samples -= 1; glUniform1i(samplesLoc, samples); std::cout << "Decrease\n"; }
			}
			if (event.type == SDL_MOUSEMOTION) {
				float dx = event.motion.xrel;
				float dy = event.motion.yrel;

				camera.dir = glm::normalize(camera.dir + camera.right * dx * sensitivity - camera.up * dy * sensitivity);
			}
		}
		if (keystate[SDL_SCANCODE_W]) camera.position += camera.dir * speed * dt;
		if (keystate[SDL_SCANCODE_S]) camera.position -= camera.dir * speed * dt;
		if (keystate[SDL_SCANCODE_A]) camera.position -= camera.right * speed * dt;
		if (keystate[SDL_SCANCODE_D]) camera.position += camera.right * speed * dt;
		if (keystate[SDL_SCANCODE_Q]) camera.position -= camera.up * speed * dt;
		if (keystate[SDL_SCANCODE_E]) camera.position += camera.up * speed * dt;
		
		scene.UpdateCamera(camera);
		glClearColor(0, 0, 0, 1);

		shader.Use();
		glUniform1i(seedLoc, rand() % 1000);
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
	scene.Delete();
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}