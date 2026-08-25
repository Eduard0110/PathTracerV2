#version 460 core

layout(binding = 0) uniform sampler2D inputTex;

uniform float exposure;
uniform float contrast;
uniform float saturation;
uniform bool useAcesToneMapping;

in vec2 texCoords;
out vec4 fragColor;

vec3 acesToneMap(vec3 col) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((col * (a * col + b)) / (col * (c * col + d) + e), 0.0, 1.0);
}

vec3 linearToSRGB(vec3 colour) {
    vec3 low = colour * 12.92;
    vec3 high = 1.055 * pow(max(colour, vec3(0.0)), vec3(1.0 / 2.4)) - 0.055;
    return mix(low, high, step(vec3(0.0031308), colour));
}

void main() {
    vec3 col = texture(inputTex, texCoords).rgb;

    col = max(col, vec3(0.0)) * exp2(exposure);

    if (useAcesToneMapping)
        col = acesToneMap(col);
    else
        col = clamp(col, 0.0, 1.0);

    float luminance = dot(col, vec3(0.2126, 0.7152, 0.0722));
    col = mix(vec3(luminance), col, max(saturation, 0.0));
    col = (col - 0.5) * max(contrast, 0.0) + 0.5;
    col = clamp(col, 0.0, 1.0);

    col = linearToSRGB(col);

    fragColor = vec4(col, 1.0);
}
