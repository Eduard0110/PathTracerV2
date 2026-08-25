#version 460 core

struct Material {
    vec4 baseColour;
    vec4 emission;

    float roughness;
    float metallic;
    float transmission;
    float ior;
};

layout(std430, binding = 4) readonly buffer MaterialBuffer {
    Material materials[];
};

uniform uint materialId;

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uv;

layout(location = 0) out vec4 fragmentColour;

void main()
{
    if (materialId >= materials.length()) {
        fragmentColour = vec4(1.0, 0.0, 1.0, 1.0);
        return;
    }

    Material material = materials[materialId];

    vec3 normal = normalize(worldNormal);
    vec3 directionToLight = normalize(vec3(-0.45, 0.85, -0.35));
    float diffuseAmount = max(dot(normal, directionToLight), 0.0);

    vec3 diffuse = material.baseColour.rgb * (0.18 + 0.82 * diffuseAmount);
    vec3 emitted = material.emission.rgb * material.emission.a;
    vec3 linearColour = diffuse + emitted;

    // Keep the raster target linear HDR. Renderer routes it through the same
    // post-process shader used by the path-traced accumulation texture.
    fragmentColour = vec4(max(linearColour, vec3(0.0)), 1.0);
}
