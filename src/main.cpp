#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "camera.h"
#include "shader.h"
#include "model.h"
#include "cshader.h"
#include "util.h"
#include "bvh.h"
#include "quad.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct Light {
    glm::vec4 position;
    glm::vec4 color;
};

std::vector<Light> lights = {
    {glm::vec4(0.0f, 5.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
};

int main() {
    if (!initializeGLFW()) {
        return -1;
    }

    GLFWwindow* window = createWindow(SCR_WIDTH, SCR_HEIGHT, "GLTF Model");
    if (!window) {
        return -1;
    }

    if (!initializeGLAD()) {
        cleanup(window);
        return -1;
    }

    // Set viewport size and callback functions
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    Model model(SOURCE_DIR "/asset/furina/scene.gltf");

    std::vector<Triangle> triangles = model.getTriangles();
    std::vector<Data> dataArray = makeData(triangles);
    BVH bvh(dataArray);
    const std::vector<BVHNode>& nodes = bvh.getNodes();
    const std::vector<Data>& data = bvh.getDataArray();
    GLuint triangleSSBO = createSSBO(data.data(), data.size() * sizeof(Data), 0);
    GLuint nodeSSBO = createSSBO(nodes.data(), nodes.size() * sizeof(BVHNode), 1);
    GLuint lightSSBO = createSSBO(lights.data(), lights.size() * sizeof(Light), 2);

    // compute_shader
    Cshader cshader(SOURCE_DIR "/src/shader/tiny_compute.glsl");

    // quad is used for to show the image computed by compute_shader
    Quad quad;

    // framebuffer for compute_shader
    GLuint framebufferTexture = createTexture(SCR_WIDTH, SCR_HEIGHT);
    GLuint framebuffer = createFramebuffer(framebufferTexture);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        cshader.use();
        cshader.setVec3("cameraPosition", camera.Position);
        cshader.setVec3("cameraFront", camera.Front);
        cshader.setVec3("cameraUp", camera.Up);
        cshader.setVec3("cameraRight", camera.Right);
        cshader.setFloat("aspectRatio", (float)SCR_WIDTH / (float)SCR_HEIGHT);
        cshader.setFloat("fov", camera.Zoom);
        cshader.setInt("numLights", (int)lights.size());

        glBindImageTexture(0, framebufferTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute((GLuint)SCR_WIDTH / 16, (GLuint)SCR_HEIGHT / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebufferTexture);
        quad.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteTextures(1, &framebufferTexture);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteBuffers(1, &triangleSSBO);
    glDeleteBuffers(1, &nodeSSBO);
    glDeleteBuffers(1, &lightSSBO);
    quad.cleanup();
    cleanup(window);
    return 0;
}

// Callback function for framebuffer size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback function for mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Callback function for mouse scroll wheel
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

// Function to process input from keyboard
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}
