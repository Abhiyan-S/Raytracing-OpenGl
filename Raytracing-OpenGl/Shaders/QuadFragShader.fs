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
	vec3 dir;
	float pad2;
	float horizontalFOV;
	float focalLength;
	float aspectRatio;
	float screenWidth;
	float screenHeight;
} cam;

struct Ray{
	vec3 origin;
	vec3 dir;
};
struct HitInfo{
	bool didHit;
	float distance;
};
HitInfo TraceSphere(Ray ray, int sIdx){
	vec3 offset = ray.origin - spheres[sIdx].position;
	float b = 2 * dot(offset, ray.dir);
	float c = dot(offset,offset) - spheres[sIdx].radius * spheres[sIdx].radius;
	float discriminant = b * b - 4 * c;

	HitInfo hit;
	hit.distance = 1000000;
	hit.didHit = false;
	if(discriminant >= 0){
		hit.didHit = true;
		float dst1 = (-b + sqrt(discriminant))/2;
		float dst2 = (-b - sqrt(discriminant))/2;

		if(dst1*dst2 < 0){
			hit.distance = max(dst1, dst2);
		}
		else{
			hit.distance = min(dst1, dst2);
		}
	}

	return hit;
}

void main(){
	vec3 currentColor = vec3(0,0,0);
	float x = (((gl_FragCoord.x - 0.5)/resolution.x) - 0.5) * 2;
	float y = (((gl_FragCoord.y - 0.5)/resolution.y) - 0.5) * 2;

	Ray ray;
	ray.origin = cam.position;
	vec3 test = cam.position;

	vec3 camRight = cross(cam.dir, vec3(0,1,0));
	vec3 camUp = cross(cam.dir, camRight);

	vec3 pointInScreen = (cam.position + (cam.dir * cam.focalLength)) + (camRight * (x * (cam.screenWidth/2))) + (camUp * (y * (cam.screenHeight/2)));

	ray.dir = normalize(pointInScreen - cam.position);
	for(int i = 0; i < spheres.length(); i++){
		HitInfo hit = TraceSphere(ray, i);
		if(hit.didHit){
			currentColor = vec3(1,1,0);
			break;
		}
		else{
			currentColor = vec3(0,0,0);
		}
	}
	FragColor = vec4(currentColor, 1);
}