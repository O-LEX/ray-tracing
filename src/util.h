#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "model.h"


bool initializeGLFW();
bool initializeGLAD();
GLFWwindow* createWindow(int width, int height, const char* title);
void cleanup(GLFWwindow* window);

GLuint createTexture(int width, int height);
GLuint createFramebuffer(GLuint texture);

struct Data {
    glm::vec4 v0;
    glm::vec4 v1;
    glm::vec4 v2;
};

GLuint createUBO(const void* data, GLsizeiptr size, GLuint binding);
GLuint createSSBO(const void* data, size_t size, GLuint binding);
std::vector<Data> makeData(const std::vector<Triangle>& triangles);
