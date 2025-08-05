#version 460 core

layout(binding = 0) uniform sampler2D computeTex;
layout(binding = 1) uniform sampler2D accumTex;

uniform float framesStill;

in vec2 texCoords;

out vec4 fragColor;

void main() {
    vec3 computeTexCol = texture(computeTex, texCoords).rgb;
    vec3 accumTexCol = texture(accumTex, texCoords).rgb;

    fragColor = vec4(mix(accumTexCol, computeTexCol, 1.0 / framesStill), 1.0);
}