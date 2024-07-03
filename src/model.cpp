#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "model.h"
#include <iostream>

Model::Model(const std::string &path) {
    loadModel(path);
}

void Model::Draw(Shader &shader) {
    for (auto &mesh : meshes) {
        if (mesh.textureID != 0) {
            shader.setTexture("texture_diffuse", 0, mesh.textureID);
        }

        glBindVertexArray(mesh.VAO);
        if (!mesh.indices.empty()) {
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertices.size() / 5); // Assuming 5 floats per vertex (3 for position, 2 for texCoord)
        }
        glBindVertexArray(0);
    }
}

void Model::loadModel(const std::string &path) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    if (!warn.empty()) {
        std::cerr << "WARN: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "ERR: " << err << std::endl;
    }
    if (!ret) {
        throw std::runtime_error("Failed to load GLTF model");
    }

    for (const auto &node : model.nodes) {
        processNode(model, const_cast<tinygltf::Node &>(node));
    }
}

void Model::processNode(tinygltf::Model &model, tinygltf::Node &node) {
    if (node.mesh >= 0) {
        meshes.push_back(processMesh(model, model.meshes[node.mesh]));
    }

    for (int childIndex : node.children) {
        processNode(model, model.nodes[childIndex]);
    }
}

Model::Mesh Model::processMesh(tinygltf::Model &model, tinygltf::Mesh &mesh) {
    Mesh loadedMesh;

    for (auto &primitive : mesh.primitives) {
        // Positions
        const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes["POSITION"]];
        const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
        const tinygltf::Buffer &posBuffer = model.buffers[posView.buffer];

        // Texture Coordinates
        const tinygltf::Accessor &texAccessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
        const tinygltf::BufferView &texView = model.bufferViews[texAccessor.bufferView];
        const tinygltf::Buffer &texBuffer = model.buffers[texView.buffer];

        const float *positions = reinterpret_cast<const float*>(
            posBuffer.data.data() + posView.byteOffset + posAccessor.byteOffset);
        const float *texCoords = reinterpret_cast<const float*>(
            texBuffer.data.data() + texView.byteOffset + texAccessor.byteOffset);

        for (size_t i = 0; i < posAccessor.count; ++i) {
            loadedMesh.vertices.push_back(positions[i * 3]);
            loadedMesh.vertices.push_back(positions[i * 3 + 1]);
            loadedMesh.vertices.push_back(positions[i * 3 + 2]);

            loadedMesh.vertices.push_back(texCoords[i * 2]);
            loadedMesh.vertices.push_back(texCoords[i * 2 + 1]);
        }

        // Indices
        if (primitive.indices >= 0) {
            const tinygltf::Accessor &idxAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView &idxView = model.bufferViews[idxAccessor.bufferView];
            const tinygltf::Buffer &idxBuffer = model.buffers[idxView.buffer];

            const unsigned int *indices = reinterpret_cast<const unsigned int*>(
                idxBuffer.data.data() + idxView.byteOffset + idxAccessor.byteOffset);

            loadedMesh.indices.insert(loadedMesh.indices.end(), indices, indices + idxAccessor.count);
        }

        // Texture
        if (primitive.material >= 0) {
            const tinygltf::Material &mat = model.materials[primitive.material];
            if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                loadedMesh.textureID = loadTexture(model, mat.pbrMetallicRoughness.baseColorTexture.index);
            }
        }
    }

    glGenVertexArrays(1, &loadedMesh.VAO);
    glGenBuffers(1, &loadedMesh.VBO);
    if (!loadedMesh.indices.empty()) glGenBuffers(1, &loadedMesh.EBO);

    glBindVertexArray(loadedMesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, loadedMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, loadedMesh.vertices.size() * sizeof(float), &loadedMesh.vertices[0], GL_STATIC_DRAW);

    if (!loadedMesh.indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, loadedMesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, loadedMesh.indices.size() * sizeof(unsigned int), &loadedMesh.indices[0], GL_STATIC_DRAW);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return loadedMesh;
}

GLuint Model::loadTexture(tinygltf::Model &model, int texIndex) {
    if (texIndex < 0) return 0;

    GLuint textureID;
    glGenTextures(1, &textureID);

    const tinygltf::Texture &tex = model.textures[texIndex];
    const tinygltf::Image &image = model.images[tex.source];

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image.image[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

std::vector<Triangle> Model::getTriangles() const {
    std::vector<Triangle> triangles;

    for (const auto& mesh : meshes) {
        const auto& vertices = mesh.vertices;
        const auto& indices = mesh.indices;

        if (!indices.empty()) {
            for (size_t i = 0; i < indices.size(); i += 3) {
                triangles.push_back({ 
                    glm::vec3(vertices[indices[i] * 5], vertices[indices[i] * 5 + 1], vertices[indices[i] * 5 + 2]),
                    glm::vec3(vertices[indices[i+1] * 5], vertices[indices[i+1] * 5 + 1], vertices[indices[i+1] * 5 + 2]),
                    glm::vec3(vertices[indices[i+2] * 5], vertices[indices[i+2] * 5 + 1], vertices[indices[i+2] * 5 + 2])
                });
            }
        } else {
            for (size_t i = 0; i < vertices.size() / 5; i += 3) {
                triangles.push_back({ 
                    glm::vec3(vertices[i * 5], vertices[i * 5 + 1], vertices[i * 5 + 2]),
                    glm::vec3(vertices[(i+1) * 5], vertices[(i+1) * 5 + 1], vertices[(i+1) * 5 + 2]),
                    glm::vec3(vertices[(i+2) * 5], vertices[(i+2) * 5 + 1], vertices[(i+2) * 5 + 2])
                });
            }
        }
    }

    return triangles;
}