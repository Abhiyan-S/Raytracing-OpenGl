#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include<SDL.h>
#include<glad/glad.h>
#include "Shaders/Shader.h"
#include "Scene.h"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include "Objects.h"
#include<vector>
#include<iostream>
#include<chrono>
#include<string>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

bool running = true;
int width = 800, height = 600;
float dt=0;

enum RenderMode {
	RAYTRACING, NORMAL
};

void CreateScreenQuad(GLuint *vao, GLuint *vbo) {
	float verts[] = {
		-1,1, 0, 1,
		 1,1, 1, 1,
		 1,-1, 1, 0,
		 1,-1, 1, 0,
		 -1,-1, 0,0,
		 -1,1, 0, 1
	};
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

std::vector<Sphere> SetSpheres() {
	std::vector<Sphere> spheres = {
									Sphere(glm::vec3(0,-102,2), 100),
									Sphere(glm::vec3(0,1,0), 3),
									Sphere(glm::vec3(6,0,0), 2),
									Sphere(glm::vec3(10,0,0), 1),
									Sphere(glm::vec3(0,20,0), 10)
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
	spheres[3].material.color = glm::vec3(0.1, 0.1, 1);

	return spheres;
}


void ClearTexture(GLuint texture) {
	GLuint tempFBO;
	glGenFramebuffers(1, &tempFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDeleteFramebuffers(1, &tempFBO);
}

void ResetTextures(GLuint accumTex[], int *frameCount) {
	*frameCount = 1;
	ClearTexture(accumTex[0]);
	ClearTexture(accumTex[1]);
}

void HandleCameraMovement(const Uint8* keystate, Camera* camera) {
	if (keystate[SDL_SCANCODE_W]) camera->position += camera->dir * camera->speed * dt;
	if (keystate[SDL_SCANCODE_S]) camera->position -= camera->dir * camera->speed * dt;
	if (keystate[SDL_SCANCODE_A]) camera->position -= camera->right * camera->speed * dt;
	if (keystate[SDL_SCANCODE_D]) camera->position += camera->right * camera->speed * dt;
	if (keystate[SDL_SCANCODE_Q]) camera->position -= camera->up * camera->speed * dt;
	if (keystate[SDL_SCANCODE_E]) camera->position += camera->up * camera->speed * dt;
}

void HandleMouseMotion(float dx, float dy, Camera *camera) {
	camera->dir = glm::normalize(camera->dir + camera->right * dx * camera->sensitivity - camera->up * dy * camera->sensitivity);
}

void GenerateGUI() {

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
	Shader accumShader("Shaders/QuadVertexShader.vs", "Shaders/AccumShader.fs");
	Shader displayShader("Shaders/QuadVertexShader.vs", "Shaders/DisplayShader.fs");

	int resLoc = glGetUniformLocation(shader.ID, "resolution");

	Camera camera(glm::vec3(5, 5, 15), glm::vec3(0, 0, -1), 60, 5, width / (float)height);
	camera.speed = 10;
	camera.sensitivity = 0.001;

	Scene scene(shader);
	scene.UpdateCamera(camera);

	std::vector<Sphere> spheres = SetSpheres();

	GLuint ssbo_lights;
	std::vector<Light> lights = {
									Light(glm::vec3(7,5,2), glm::vec3(1,1,1),0.5),
									Light(glm::vec3(2,5,8), glm::vec3(0,0.4,0.3),0.25)
								   };

	std::vector<Object> objects = {
									Object(glm::vec3(0,0,0), 1)
	};
	objects[0].AddTriangle(Triangle(glm::vec3(-10, 10, 0), glm::vec3(0, 10, 10), glm::vec3(10, 10, 10)));
	objects[0].material.color = glm::vec3(1, 1, 1);

	scene.UpdateSpheres(spheres);
	scene.UpdateLights(lights);
	scene.UpdateObjects(objects);

	GLuint sceneFBO, sceneTex;
	glGenFramebuffers(1, &sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

	glGenTextures(1, &sceneTex);
	glBindTexture(GL_TEXTURE_2D, sceneTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneTex, 0);

	GLuint accumFBO[2], accumTex[2];
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

	float time = 0;

	int frameCount = 1;
	int current = 0, next = 1;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	
	RenderMode renderMode = NORMAL;

	GLuint seedLoc = glGetUniformLocation(shader.ID, "frameSeed");
	GLuint samplesLoc = glGetUniformLocation(shader.ID, "SAMPLES");
	GLuint bouncesLoc = glGetUniformLocation(shader.ID, "BOUNCES");
	int bounces = 5;
	int samples = 5;

	bool mouseLocked = false;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 430");

	while (running) {
		auto start = std::chrono::high_resolution_clock::now();
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT) running = false;
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
				if (event.key.keysym.sym == SDLK_RIGHT) { samples += 1; glUniform1i(samplesLoc, samples); std::cout << "Samples set to "<<samples<<"\n"; }
				if (event.key.keysym.sym == SDLK_LEFT) { samples -= 1; glUniform1i(samplesLoc, samples); std::cout << "Samples set to " << samples << "\n"; }

				if (event.key.keysym.sym == SDLK_UP) { bounces += 1; glUniform1i(bouncesLoc, bounces); std::cout << "Bounces set to " << bounces << "\n"; }
				if (event.key.keysym.sym == SDLK_DOWN) { bounces -= 1; glUniform1i(bouncesLoc, bounces); std::cout << "Bounces set to " << bounces << "\n"; }

				if(event.key.keysym.sym == SDLK_LCTRL) SDL_SetRelativeMouseMode((mouseLocked = !mouseLocked)? SDL_TRUE:SDL_FALSE);
				ResetTextures(accumTex, &frameCount);
			}
			if (event.type == SDL_MOUSEMOTION) {
				HandleMouseMotion(event.motion.xrel, event.motion.yrel, &camera);
				ResetTextures(accumTex, &frameCount);
			}
			if (event.type == SDL_KEYUP) {
				ResetTextures(accumTex, &frameCount);
			}
		}
		HandleCameraMovement(keystate, &camera);
		scene.UpdateCamera(camera);
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("GUI:");
		if (ImGui::Button("Toggle Raytracing")) {
			if (renderMode == NORMAL) renderMode = RAYTRACING; else renderMode = NORMAL;
			
		}
		std::string fps_string = std::string("FPS: ") + std::to_string(1/dt);
		ImGui::Text( fps_string.c_str());

		ImGui::Text("Press LCTRL to toggle mouse lock");
		ImGui::End();

		ImGui::Render();
		
		glClearColor(0, 0, 0, 1);

		glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();
		glUniform1i(seedLoc, rand() % 1000);
		glUniform1i(samplesLoc, samples);
		glUniform1i(bouncesLoc, bounces);

		glBindVertexArray(vao_quad);
		glUniform2f(resLoc, width, height);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_FRAMEBUFFER, accumFBO[next]);
		glClear(GL_COLOR_BUFFER_BIT);

		accumShader.Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneTex);
		glUniform1i(glGetUniformLocation(accumShader.ID, "currentFrame"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, accumTex[current]);
		glUniform1i(glGetUniformLocation(accumShader.ID, "previousAccum"), 1);

		float blendFactor = 1.0f / frameCount;
		glUniform1f(glGetUniformLocation(accumShader.ID, "blendFactor"), blendFactor);

		glBindVertexArray(vao_quad);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		displayShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, accumTex[next]);
		glUniform1i(glGetUniformLocation(displayShader.ID, "image"), 0);

		glBindVertexArray(vao_quad);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		

		std::swap(current, next);
		frameCount++;
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);

		auto end = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000000.0;
		time += dt;
	}
	scene.Delete();
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}