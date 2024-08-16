#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tiny_gltf.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct Material {
    glm::vec4 baseColor;
    float metallic;
    float roughness;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Material material;
};

class 