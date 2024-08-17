#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "load.h"
#include <iostream>

Model::Model(const std::string &path) {
    loadModel(path);
}

void Model::processPrimitive(tinygltf::Model& model, const tinygltf::Primitive& primitive) {
    Model::primitive p;

    // Positions
    const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
    const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
    const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];

    // Texture Coordinates
    const tinygltf::Accessor& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
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

    if (primitive.indices >= 0) {
        const tinygltf::Accessor& idxAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView& idxView = model.bufferViews[idxAccessor.bufferView];
        const tinygltf::Buffer& idxBuffer = model.buffers[idxView.buffer];

        const unsigned int* indices = reinterpret_cast<const unsigned int*>(
            idxBuffer.data.data() + idxView.byteOffset + idxAccessor.byteOffset);
        p.indices = std::vector<uint32_t>(indices, indices + idxAccessor.count);
    }
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