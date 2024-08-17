#pragma once

#include <string>
#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include "tiny_gltf.h"

class Model {
public:
    struct vertex {
        glm::vec3 position;
        glm::vec2 texCoord;
    };

    struct primitive {
        std::vector<vertex> vertices;
        std::vector<uint32_t> indices;
        GLuint textureID;
    };

    Model(const std::string &path);

private:
    std::unordered_map<int, GLuint> textureCache;
    std::vector<primitive> primitives;

    void loadModel(const std::string& path);
    GLuint loadTexture(tinygltf::Model& model, int texIndex);
    void processScene(tinygltf::Model& model, const tinygltf::Scene& scene);
    void processNode(tinygltf::Model& model, const tinygltf::Node& node);
    void processMesh(tinygltf::Model& model, const tinygltf::Mesh& mesh);
    void processPrimitive(tinygltf::Model& model, const tinygltf::Primitive& primitive);
};