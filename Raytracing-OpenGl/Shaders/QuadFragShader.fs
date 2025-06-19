#version 430 core

out vec4 FragColor;
uniform vec2 resolution;

uniform int frameSeed;
float fSeed = 0;

struct Material{
	vec3 color;
	float pad1;
	float roughness;
	bool emits;
	float emissionStrength;
	float pad2;
	//Aligning to multiple of 16bytes
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
};

layout(std430, binding = 2) buffer Spheres{
	Sphere spheres[];
};

layout(std430, binding = 1) buffer Lights{
	Light lights[];
};

layout(std140, binding = 0) uniform Camera{
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
	vec3 normal;
	vec3 point;
	Material material;
};



HitInfo TraceSphere(Ray ray, int sIdx){
	vec3 offset = ray.origin - spheres[sIdx].position;
	float b = 2 * dot(offset, ray.dir);
	float c = dot(offset,offset) - spheres[sIdx].radius * spheres[sIdx].radius;
	float discriminant = b * b - 4 * c;

	HitInfo hit;
	hit.distance = 1.0/0.0;
	hit.didHit = false;
	if(discriminant >= 0){
		
		float s = sqrt(discriminant);
		float dst1 = (-b + s)/2;
		float dst2 = (-b - s)/2;

		if(dst1*dst2 < 0){
			hit.didHit = true;
			hit.distance = max(dst1, dst2);
			hit.point = ray.origin + ray.dir * hit.distance;
			hit.normal = normalize(hit.point - spheres[sIdx].position);
			hit.material = spheres[sIdx].material;
		}
		else if(dst1 < 0 && dst2 < 0){
			hit.didHit = false;
		}
		else{
			hit.didHit = true;
			hit.distance = min(dst1, dst2);
			hit.point = ray.origin + ray.dir * hit.distance;
			hit.normal = normalize(hit.point - spheres[sIdx].position);
			hit.material = spheres[sIdx].material;
		}
	}

	return hit;
}
vec3 GetLight(vec3 point, vec3 normal){
	vec3 currentColor = vec3(0.1,0.16,0.18);
	Ray ray;
	ray.origin = point;
	for(int l = 0; l<lights.length(); l++){
		ray.dir = normalize(lights[l].position - point);
		float d = length(lights[l].position - point);
		bool blocked = false;
		HitInfo hit;
		for(int i = 0; i<spheres.length(); i++){
			hit = TraceSphere(ray, i);
			if(hit.didHit){
				if(hit.distance < d){
					blocked = true;
					break;
				}
			}
		}
		if(!blocked){
			currentColor += (lights[l].color * lights[l].intensity * dot(normal, ray.dir));
		}
	}
	return currentColor;
}

float Random() {
	vec2 co = gl_FragCoord.xy;
	fSeed++;
    return fract(sin(dot(co, vec2(12.9898, 78.233)) +fSeed + frameSeed) * 43758.5453);
}

vec3 RandomVectorInHemisphere(vec3 normal){
	vec3 rand = normalize(vec3((Random()-0.5)*2,(Random()-0.5)*2,(Random()-0.5)*2));
	if(dot(rand, normal) < 0){
		return -rand;
	}
	return rand;
}

HitInfo TracePath(Ray ray){
	HitInfo closestHitInfo;
	closestHitInfo.didHit = false;
	closestHitInfo.distance = 1.0/0.0;

	for(int i = 0; i < spheres.length(); i++){
		HitInfo hit = TraceSphere(ray, i);
		if(hit.didHit){
			if(hit.distance > 0.0001 && hit.distance < closestHitInfo.distance){
				closestHitInfo = hit;
				
			}
		}
	}
	return closestHitInfo;
}
#define BOUNCES 12

uniform int SAMPLES;

void main(){
	
	float x = (((gl_FragCoord.x)/resolution.x) - 0.5) * 2;
	float y = (((gl_FragCoord.y)/resolution.y) - 0.5) * 2;

	Ray ray;
	
	vec3 test = cam.position;


	vec3 pointInScreen = (cam.position + (cam.dir * cam.focalLength)) + (cam.right * (x * (cam.screenWidth/2))) + (cam.up * (y * (cam.screenHeight/2)));

	vec3 currentColor = vec3(0.529, 0.808, 0.922);
	vec3 acolor = vec3(0);
	
	for(int s=0; s<SAMPLES; s++){
		vec3 rayColor = vec3(1,1,1);
		ray.origin = cam.position;
		ray.dir = normalize(pointInScreen - cam.position);
	for(int bounce=0; bounce < BOUNCES; bounce++){
		HitInfo hit = TracePath(ray);
		if(hit.didHit){
			if(hit.material.emits) {acolor += hit.material.color * hit.material.emissionStrength;}
			else acolor += hit.material.color * GetLight(hit.point + hit.normal * 0.001, hit.normal) * hit.material.roughness;
			
		}
		else{
			break;
		}
		ray.origin = hit.point;

		vec3 diffuse = RandomVectorInHemisphere(hit.normal);

		vec3 specular = ray.dir - 2 * dot(ray.dir,hit.normal) * hit.normal;
		vec3 AB = specular - diffuse;
		ray.dir = diffuse + AB * (1-hit.material.roughness);
	}
	}
	FragColor = vec4(acolor/SAMPLES, 1);

}