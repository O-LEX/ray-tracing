#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "load.h"
#include <iostream>

Model::Model(const std::string &path) {
    loadModel(path);
}