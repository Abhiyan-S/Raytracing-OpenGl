#version 430 core

layout (location = 0) in vec2 pos;
void main(){
	gl_Position = vec4(pos.xy, 1, 1);
}