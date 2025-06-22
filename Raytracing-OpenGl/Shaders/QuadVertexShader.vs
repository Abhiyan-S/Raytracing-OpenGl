#version 430 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
void main(){
	gl_Position = vec4(pos.xy, 1, 1);
	TexCoord = aTexCoord;
}