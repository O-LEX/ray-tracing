#pragma once

#include <string>
#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include "tiny_gltf.h"

struct vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
};

struct primitive {
    std::vector<vertex> vertices;
    std::vector<uint32_t> indices;
    int textureIndex; // not textureID
};

class Model {
public:
    void loadModel(const std::string& path);
    const std::vector<primitive>& getPrimitives() const { return primitives; }

    Model(const std::string& path);
private:
    std::unordered_map<int, GLuint> textureCache;
    std::vector<primitive> primitives;

    GLuint loadTexture(tinygltf::Model& model, int texIndex);
    void processScene(tinygltf::Model& model, const tinygltf::Scene& scene);
    void processNode(tinygltf::Model& model, const tinygltf::Node& node);
    void processMesh(tinygltf::Model& model, const tinygltf::Mesh& mesh);
    void processPrimitive(tinygltf::Model& model, const tinygltf::Primitive& primitive);
};

struct Triangle {
    glm::vec3 v0, v1, v2; // vertex
    glm::vec2 t0, t1, t2; // texture
    int textureIndex;
};

struct BVHNode {
    glm::vec3 bounds[2];  // AABB
    BVHNode* left;
    BVHNode* right;
    int triangleIndex;    // -1 ならリーフでない
};

std::vector<Triangle> extractTriangles(const std::vector<primitive>& primitives);
BVHNode* buildBVH(std::vector<Triangle>& triangles, int start, int end);
BVHNode* generateBVH(const std::vector<primitive>& primitives);
void freeBVH(BVHNode* node);
