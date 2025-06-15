#version 430 core

out vec4 FragColor;
uniform vec2 resolution;

struct Sphere{
	vec3 position;
	float radius;
};

layout(std430, binding = 0) buffer Spheres{
	Sphere spheres[];
};

layout(std140, binding = 1) uniform Camera{
	vec3 position;
	float pad1;
	vec3 direction;
	float pad2;
	float horizontalFOV;
	float focalLength;
	float aspectRatio;
	float screenWidth;
	float screenHeight;
};

void main(){
	vec3 currentColor = vec3(0,0,0);
	float x = (((gl_FragCoord.x - 0.5)/resolution.x) - 0.5) * 2;
	float y = (((gl_FragCoord.y - 0.5)/resolution.y) - 0.5) * 2;

	for(int i = 0; i < spheres.length(); i++){
		float dist = length(vec2(x,y) - spheres[i].position.xy);
		if(dist < spheres[i].radius){
			currentColor = vec3(1,0,0);
			break;
		}
	}
	FragColor = vec4(currentColor, 1);
}