#ifndef QUAD_H
#define QUAD_H

#include <glad/gl.h>
#include "shader.h" 

class Quad {
public:
    Quad(const char* vertexPath = SOURCE_DIR "/src/shader/simple_vertex.glsl",
     const char* fragmentPath = SOURCE_DIR "/src/shader/simple_fragment.glsl");
    void cleanup();
    void draw();

private:
    GLuint quadVAO, quadVBO;
    Shader shader;
    void setupQuad();

    const char* vertexPath;
    const char* fragmentPath;
};

#endif
