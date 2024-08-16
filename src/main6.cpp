#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <iostream>

int main(int argc, char** argv) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, SOURCE_DIR "/asset/furina/scene.gltf");

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!ret) {
        std::cout << "Failed to parse glTF" << std::endl;
        return -1;
    }

    // モデルデータにアクセスする
    std::cout << "Scenes: " << model.scenes.size() << std::endl;
    std::cout << "Nodes: " << model.nodes.size() << std::endl;
    std::cout << "Meshes: " << model.meshes.size() << std::endl;
    std::cout << "Materials: " << model.materials.size() << std::endl;
    std::cout << "Textures: " << model.textures.size() << std::endl;
    
    return 0;
}