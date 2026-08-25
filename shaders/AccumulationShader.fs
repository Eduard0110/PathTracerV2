#version 460 core

layout(binding = 0) uniform sampler2D computeTex;
layout(binding = 1) uniform sampler2D accumTex;

uniform float accumulatedFrameCount;

in vec2 texCoords;

out vec4 fragColor;

void main() {
    vec3 computeTexCol = texture(computeTex, texCoords).rgb;

    // A newly created accumulation texture has undefined contents. The first
    // frame must therefore copy the compute result without sampling it.
    if (accumulatedFrameCount <= 1.0) {
        fragColor = vec4(computeTexCol, 1.0);
        return;
    }

    vec3 accumTexCol = texture(accumTex, texCoords).rgb;
    fragColor = vec4(mix(accumTexCol, computeTexCol, 1.0 / accumulatedFrameCount), 1.0);
}
