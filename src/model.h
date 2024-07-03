#ifndef MODEL_H
#define MODEL_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <tiny_gltf.h>
#include <string>
#include <vector>
#include "shader.h"

struct Triangle {
    glm::vec3 v0, v1, v2;
};

class Model {
public:
    Model(const std::string &path);
    void Draw(Shader &shader);

    std::vector<Triangle> getTriangles() const;

private:
    struct Mesh {
        GLuint VAO, VBO, EBO, textureID;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
    };

    std::vector<Mesh> meshes;
    void loadModel(const std::string &path);
    void processNode(tinygltf::Model &model, tinygltf::Node &node);
    Mesh processMesh(tinygltf::Model &model, tinygltf::Mesh &mesh);
    GLuint loadTexture(tinygltf::Model &model, int texIndex);
};

#endif