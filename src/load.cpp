#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "load.h"
#include <iostream>

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::processPrimitive(tinygltf::Model& model, const tinygltf::Primitive& prim) {
    primitive p;

    // Positions
    const tinygltf::Accessor& posAccessor = model.accessors[prim.attributes.at("POSITION")];
    const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
    const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];

    // Texture Coordinates
    const tinygltf::Accessor& texAccessor = model.accessors[prim.attributes.at("TEXCOORD_0")];
    const tinygltf::BufferView& texView = model.bufferViews[texAccessor.bufferView];
    const tinygltf::Buffer& texBuffer = model.buffers[texView.buffer];

    const float* positions = reinterpret_cast<const float*>(
        posBuffer.data.data() + posView.byteOffset + posAccessor.byteOffset);
    const float* texCoords = reinterpret_cast<const float*>(
        texBuffer.data.data() + texView.byteOffset + texAccessor.byteOffset);
    
    for (size_t i = 0; i < posAccessor.count; ++i) {
        vertex v;
        v.position = {positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]};
        v.texCoord = {texCoords[i * 2], texCoords[i * 2 + 1]};
        p.vertices.push_back(v);
    }

    if (prim.indices >= 0) {
        const tinygltf::Accessor& idxAccessor = model.accessors[prim.indices];
        const tinygltf::BufferView& idxView = model.bufferViews[idxAccessor.bufferView];
        const tinygltf::Buffer& idxBuffer = model.buffers[idxView.buffer];

        const unsigned int* indices = reinterpret_cast<const unsigned int*>(
            idxBuffer.data.data() + idxView.byteOffset + idxAccessor.byteOffset);
        p.indices = std::vector<uint32_t>(indices, indices + idxAccessor.count);
    }

    if (prim.material >= 0) {
        const tinygltf::Material& mat = model.materials[prim.material];
        if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            p.textureIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
        }
    }

    primitives.push_back(p);
}

GLuint Model::loadTexture(tinygltf::Model& model, int texIndex) {
    if (texIndex < 0) return 0;

    if (textureCache.find(texIndex) != textureCache.end()) {
        return textureCache[texIndex];
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    const tinygltf::Texture& tex = model.textures[texIndex];
    const tinygltf::Image& image = model.images[tex.source];

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image.image[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    textureCache[texIndex] = textureID;

    return textureID;
}

void Model::processMesh(tinygltf::Model& model, const tinygltf::Mesh& mesh) {
    for (auto& primitive : mesh.primitives) {
        processPrimitive(model, primitive);
    }
}

void Model::processNode(tinygltf::Model& model, const tinygltf::Node& node) {
    if (node.mesh >= 0) {
        processMesh(model, model.meshes[node.mesh]);
    }

    for (int childIndex : node.children) {
        processNode(model, model.nodes[childIndex]);
    }
}

void Model::processScene(tinygltf::Model& model, const tinygltf::Scene& scene) {
    for (const auto& node : scene.nodes) {
        processNode(model, model.nodes[node]);
    }
}

void Model::loadModel(const std::string& path) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!ret) {
        std::cout << "Failed to parse glTF" << std::endl;
    }

    for (const auto& scene : model.scenes) {
        processScene(model, scene);
    }
}

std::vector<Triangle> extractTriangles(const std::vector<primitive>& primitives) {
    std::vector<Triangle> triangles;
    for (const auto& prim : primitives) {
        for (size_t i = 0; i < prim.indices.size(); i += 3) {
            Triangle tri;
            tri.v0 = prim.vertices[prim.indices[i]].position;
            tri.v1 = prim.vertices[prim.indices[i+1]].position;
            tri.v2 = prim.vertices[prim.indices[i+2]].position;
            tri.t0 = prim.vertices[prim.indices[i]].texCoord;
            tri.t1 = prim.vertices[prim.indices[i+1]].texCoord;
            tri.t2 = prim.vertices[prim.indices[i+2]].texCoord;
            tri.textureIndex = prim.textureIndex;
            triangles.push_back(tri);
        }
    }
    return triangles;
}

BVHNode* buildBVH(std::vector<Triangle>& triangles, int start, int end) {
    if (start >= end) return nullptr;

    BVHNode* node = new BVHNode();

    glm::vec3 minBound(FLT_MAX), maxBound(-FLT_MAX);
    for (int i = start; i < end; ++i) {
        const Triangle& tri = triangles[i];
        for (const glm::vec3& v : {tri.v0, tri.v1, tri.v2}) {
            minBound = glm::min(minBound, v);
            maxBound = glm::max(maxBound, v);
        }
    }
    node->bounds[0] = minBound;
    node->bounds[1] = maxBound;

    if (end - start == 1) {
        node->triangleIndex = start;
        node->left = node->right = nullptr;
        return node;
    }

    int axis = 0;
    glm::vec3 extent = maxBound - minBound;
    if (extent.y > extent.x && extent.y > extent.z) {
        axis = 1;
    } else if (extent.z > extent.x) {
        axis = 2;
    }

    int mid = (start + end) / 2;
    std::nth_element(triangles.begin() + start, triangles.begin() + mid, triangles.begin() + end,
                     [axis](const Triangle& a, const Triangle& b) {
                         return (a.v0[axis] + a.v1[axis] + a.v2[axis]) <
                                (b.v0[axis] + b.v1[axis] + b.v2[axis]);
                     });

    node->left = buildBVH(triangles, start, mid);
    node->right = buildBVH(triangles, mid, end);
    node->triangleIndex = -1;

    return node;
}

BVHNode* generateBVH(const std::vector<primitive>& primitives) {
    std::vector<Triangle> triangles = extractTriangles(primitives);
    return buildBVH(triangles, 0, triangles.size());
}

void freeBVH(BVHNode* node) {
    if (!node) return;
    freeBVH(node->left);
    freeBVH(node->right);
    delete node;
}