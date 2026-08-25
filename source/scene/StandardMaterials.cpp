#include "scene/StandardMaterials.hpp"

std::vector<Material> createStandardMaterials() {
    Material gold;
    gold.name = "Polished gold";
    gold.baseColour = { 1.0f, 0.71f, 0.29f };
    gold.roughness = 0.12f;
    gold.metallic = 1.0f;
    gold.ior = 1.5f;

    Material copper;
    copper.name = "Brushed copper";
    copper.baseColour = { 0.95f, 0.42f, 0.19f };
    copper.roughness = 0.48f;
    copper.metallic = 1.0f;
    copper.ior = 1.5f;

    Material plastic;
    plastic.name = "Red plastic";
    plastic.baseColour = { 0.8f, 0.025f, 0.02f };
    plastic.roughness = 0.32f;
    plastic.metallic = 0.0f;
    plastic.ior = 1.48f;

    Material ceramic;
    ceramic.name = "High-IOR blue ceramic";
    ceramic.baseColour = { 0.03f, 0.18f, 0.8f };
    ceramic.roughness = 0.07f;
    ceramic.metallic = 0.0f;
    ceramic.ior = 2.2f;

    Material light;
    light.name = "Warm emissive light";
    light.baseColour = { 0.02f, 0.02f, 0.02f };
    light.emission = { 1.0f, 0.72f, 0.42f };
    light.emissionStrength = 18.0f;
    light.roughness = 1.0f;
    light.metallic = 0.0f;
    light.ior = 1.5f;

    return { gold, copper, plastic, ceramic, light };
}
