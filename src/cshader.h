#ifndef CSHADER_H
#define CSHADER_H

#include <glad/gl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

class Cshader {
public:
    GLuint ID;

    Cshader(const char* computePath);
    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setTexture(const std::string &name, int unit, GLuint texture);

    void dispatch(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
     void waitForCompute();

private:
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif