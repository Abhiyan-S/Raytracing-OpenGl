#version 430 core

in vec2 TexCoord;

uniform sampler2D currentFrame;
uniform sampler2D previousAccum;
uniform float blendFactor;

out vec4 FragColor;

void main(){
	vec3 curr = texture(currentFrame, TexCoord).rgb;
    vec3 prev = texture(previousAccum, TexCoord).rgb;

    vec3 blended = mix(prev, curr, blendFactor);

    FragColor = vec4(blended, 1.0);
}