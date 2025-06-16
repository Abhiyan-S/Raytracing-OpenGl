#version 430 core

out vec4 FragColor;
uniform vec2 resolution;

struct Material{
	vec3 color;
	float roughness;
};

struct Sphere{
	vec3 position;
	float radius;
	Material material;
};

struct Light{
	vec3 position;
	float pad1;
	vec3 color;
	float intensity;
}

layout(std430, binding = 1) buffer Spheres{
	Sphere spheres[];
};

layout(std140, binding = 1) uniform Camera{
	vec3 position;
	float pad1;
	vec3 dir;
	float pad2;
	vec3 up;
	float pad3;
	vec3 right;
	float pad4;
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
	hit.distance = 1.0/0/0;
	hit.didHit = false;
	if(discriminant >= 0){
		
		float s = sqrt(discriminant);
		float dst1 = (-b + s)/2;
		float dst2 = (-b - s)/2;

		if(dst1*dst2 < 0){
			hit.distance = max(dst1, dst2);
			hit.didHit = true;
		}
		else if(dst1 < 0 && dst2 < 0){
			hit.didHit = false;
			hit.distance = min(dst1, dst2);
		}
		else{
			hit.didHit = true;
			hit.distance = min(dst1, dst2);
		}
	}

	return hit;
}

float GetLightIntensity(vec3 point){

}

void main(){
	vec3 currentColor = vec3(0.529, 0.808, 0.922);
	float x = (((gl_FragCoord.x - 0.5)/resolution.x) - 0.5) * 2;
	float y = (((gl_FragCoord.y - 0.5)/resolution.y) - 0.5) * 2;

	Ray ray;
	ray.origin = cam.position;
	vec3 test = cam.position;


	vec3 pointInScreen = (cam.position + (cam.dir * cam.focalLength)) + (cam.right * (x * (cam.screenWidth/2))) + (cam.up * (y * (cam.screenHeight/2)));

	ray.dir = normalize(pointInScreen - cam.position);

	HitInfo closestHitInfo;
	closestHitInfo.didHit = false;
	closestHitInfo.distance = 1.0/0.0;

	for(int i = 0; i < spheres.length(); i++){
		HitInfo hit = TraceSphere(ray, i);
		if(hit.didHit){
			if(hit.distance < closestHitInfo.distance){
				closestHitInfo.didHit = true;
				currentColor = spheres[i].material.color;
				closestHitInfo.distance = hit.distance;
			}
		}
	}
	FragColor = vec4(currentColor, 1);
}