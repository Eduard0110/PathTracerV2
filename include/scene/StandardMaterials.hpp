#pragma once

#include <vector>

#include "scene/Geometry.hpp"

// Returns fresh Material values so Scene can assign its own IDs.
std::vector<Material> createStandardMaterials();
