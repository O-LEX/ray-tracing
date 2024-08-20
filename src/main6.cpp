#include "load.h"
#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>


int main(int argc, char** argv) {
    Model model(SOURCE_DIR "/asset/furina/scene.gltf");
    const std::vector<primitive>& primitives = model.getPrimitives();
    BVHNode* root = generateBVH(primitives);
    return 0;
}