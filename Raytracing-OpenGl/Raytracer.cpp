#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include<SDL.h>
#include<glad/glad.h>
#include "Shaders/Shader.h"
#include "Scene.h"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
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
	RAYTRACING, BASIC
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
	std::vector<Sphere> spheres;

	Sphere s1(glm::vec3(0, -102, 2), 100);
	s1.material.roughness = 1.0f;
	s1.material.color = glm::vec3(1, 1, 1);

	Sphere s2(glm::vec3(0, 1, 0), 3);
	s2.material.emits = true;
	s2.material.emissionStrength = 4.0f;
	s2.material.color = glm::vec3(0.8f, 0.1f, 1.0f);

	Sphere s3(glm::vec3(6, 0, 0), 2);
	s3.material.roughness = 0.1f;
	s3.material.color = glm::vec3(1.0f, 1.0f, 0.0f);

	Sphere s4(glm::vec3(10, 0, 0), 1);
	s4.material.emits = true;
	s4.material.emissionStrength = 4.0f;
	s4.material.color = glm::vec3(0.1f, 0.1f, 1.0f);

	spheres.push_back(std::move(s1));
	spheres.push_back(std::move(s2));
	spheres.push_back(std::move(s3));
	spheres.push_back(std::move(s4));

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
void printVec(const glm::vec3& v, const std::string& name = "") {
	std::cout << name << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")\n";
}
void RenderRaytracing(Scene &scene, GLuint &vao_quad, int bounces, int samples, int current, int next, int frameCount) {
	glDisable(GL_DEPTH_TEST); // Does not work with this enabled
	glBindFramebuffer(GL_FRAMEBUFFER, scene.sceneFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene.raytracingShader.Use();

	glUniform1i(glGetUniformLocation(scene.raytracingShader.ID, "frameSeed"), rand() % 1000);
	glUniform1i(glGetUniformLocation(scene.raytracingShader.ID, "BOUNCES"), bounces);
	glUniform1i(glGetUniformLocation(scene.raytracingShader.ID, "SAMPLES"), samples);
	glUniform2f(glGetUniformLocation(scene.raytracingShader.ID, "resolution"), width, height);

	glBindVertexArray(vao_quad);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, scene.accumFBO[next]);
	glClear(GL_COLOR_BUFFER_BIT);
	scene.accumShader.Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.sceneTex);
	glUniform1i(glGetUniformLocation(scene.accumShader.ID, "currentFrame"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scene.accumTex[current]);
	glUniform1i(glGetUniformLocation(scene.accumShader.ID, "previousAccum"), 1);

	float blendFactor = 1.0f / frameCount;
	glUniform1f(glGetUniformLocation(scene.accumShader.ID, "blendFactor"), blendFactor);

	glBindVertexArray(vao_quad);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	scene.displayShader.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.accumTex[next]);
	glUniform1i(glGetUniformLocation(scene.displayShader.ID, "image"), 0);

	glBindVertexArray(vao_quad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderBasic(Scene &scene, Camera &cam, Shader basicShader) {
	basicShader.Use();
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = glm::lookAt(cam.position, cam.position+cam.dir, cam.up);
	glUniformMatrix4fv(glGetUniformLocation(basicShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));


	//getting the vertical fov
	float h = sqrt(cam.focalLength * cam.focalLength + (cam.screenWidth / 2) * (cam.screenWidth / 2));
	float verticalFOV = atan(cam.screenHeight / (2 * h));
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(basicShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(proj));

	for (Sphere &sphere : scene.spheres) {
		glm::mat4 model(1.0);
		model = glm::translate(model, sphere.position);
		glUniformMatrix4fv(glGetUniformLocation(basicShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		sphere.Render();
	}
}


int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);


	SDL_Window* window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
	glViewport(0, 0, width, height);

	SDL_Event event;

	GLuint vao_quad, vbo_quad;
	CreateScreenQuad(&vao_quad, &vbo_quad);

	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "OpenGL version: " << version << std::endl;

	Shader raytracingShader("Shaders/QuadVertexShader.vs", "Shaders/QuadFragShader.fs");
	Shader accumShader("Shaders/QuadVertexShader.vs", "Shaders/AccumShader.fs");
	Shader displayShader("Shaders/QuadVertexShader.vs", "Shaders/DisplayShader.fs");

	Shader basicShader("Shaders/BasicRenderingVertexShader.vs", "Shaders/BasicRenderingFragmentShader.fs");

	int resLoc = glGetUniformLocation(raytracingShader.ID, "resolution");

	Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), 60, 5, width / (float)height);
	camera.speed = 10;
	camera.sensitivity = 0.001;
	camera.resolution = glm::vec2(width, height);

	Scene scene(width, height, raytracingShader, accumShader, displayShader);
	

	scene.spheres = std::move(SetSpheres());

	GLuint ssbo_lights;
	std::vector<Light> lights = {
									Light(glm::vec3(7,5,2), glm::vec3(1,1,1),0.5),
									Light(glm::vec3(2,5,8), glm::vec3(1,1,1),1)
								   };

	std::vector<Object> objects = {
									Object(glm::vec3(0,0,0), 1)
	};
	objects[0].AddTriangle(Triangle(glm::vec3(-10, 10, 0), glm::vec3(0, 10, 10), glm::vec3(10, 10, 10)));
	objects[0].material.color = glm::vec3(1, 1, 1);
	objects[0].material.roughness = 0.8;

	std::cout << objects[0].triangles[0].normal.x << objects[0].triangles[0].normal.y << objects[0].triangles[0].normal.z << std::endl;

	scene.UpdateSpheres(scene.spheres);
	scene.UpdateLights(lights);
	scene.UpdateObjects(objects);

	float time = 0;

	int frameCount = 1;
	int current = 0, next = 1;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	
	RenderMode renderMode = BASIC;

	int bounces = 5;
	int samples = 5;

	bool mouseLocked = false;

	glFrontFace(GL_CW); // instead of GL_CCW
	glEnable(GL_CULL_FACE);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 430");

	ResetTextures(scene.accumTex, &frameCount);

	camera.position = glm::vec3(0, 0, -3);
	camera.dir = glm::vec3(0, 0, 1);
	scene.UpdateCamera(camera);

	while (running) {
		auto start = std::chrono::high_resolution_clock::now();
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT) running = false;
			if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
				if (event.key.keysym.sym == SDLK_RIGHT) { samples += 1; std::cout << "Samples set to "<<samples<<"\n"; }
				if (event.key.keysym.sym == SDLK_LEFT) { samples -= 1; std::cout << "Samples set to " << samples << "\n"; }

				if (event.key.keysym.sym == SDLK_UP) { bounces += 1; std::cout << "Bounces set to " << bounces << "\n"; }
				if (event.key.keysym.sym == SDLK_DOWN) { bounces -= 1; std::cout << "Bounces set to " << bounces << "\n"; }

				if(event.key.keysym.sym == SDLK_LCTRL) SDL_SetRelativeMouseMode((mouseLocked = !mouseLocked)? SDL_TRUE:SDL_FALSE);
				if(renderMode == RAYTRACING) ResetTextures(scene.accumTex, &frameCount);
			}
			if (event.type == SDL_MOUSEMOTION) {
				HandleMouseMotion(event.motion.xrel, event.motion.yrel, &camera);
				if(renderMode == RAYTRACING) ResetTextures(scene.accumTex, &frameCount);
			}
			if (event.type == SDL_KEYUP) {
				if (renderMode == RAYTRACING) ResetTextures(scene.accumTex, &frameCount);
			}
		}
		HandleCameraMovement(keystate, &camera);
		scene.UpdateCamera(camera);

		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("GUI:");
		if (ImGui::Button("Toggle Raytracing")) {
			if (renderMode == BASIC) renderMode = RAYTRACING; else renderMode = BASIC;
			
		}
		std::string fps_string = std::string("FPS: ") + std::to_string(1/dt);

		ImGui::Text( fps_string.c_str());

		ImGui::Text("Press LCTRL to toggle mouse lock");
		ImGui::End();

		if (renderMode == RAYTRACING) {
			RenderRaytracing(scene, vao_quad, bounces, samples, current, next, frameCount);
		}
		else {
			RenderBasic(scene, camera, basicShader);
			

		}

		std::swap(current, next);
		frameCount++;
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);

		auto end = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000000.0;
		time += dt;
	}
	scene.Delete(scene.spheres);
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}