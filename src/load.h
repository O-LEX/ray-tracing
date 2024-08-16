#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "tiny_gltf.h"

class Model {
public:
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

    struct Primitive {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        Material material;
    };

    struct Mesh {
        std::vector<Primitive> primitives;
    };

    Model(const std::string &path);

private:
    std::vector<Mesh> meshes;

};