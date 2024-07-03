#include "cshader.h"

Cshader::Cshader(const char* computePath) {
    std::string computeCode;
    std::ifstream cShaderFile;
    cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        cShaderFile.open(computePath);
        std::stringstream cShaderStream;
        cShaderStream << cShaderFile.rdbuf();
        computeCode = cShaderStream.str();
        cShaderFile.close();
    } catch (std::ifstream::failure e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* cShaderCode = computeCode.c_str();

    // コンピュートシェーダーをコンパイル
    GLuint compute;
    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &cShaderCode, nullptr);
    glCompileShader(compute);
    checkCompileErrors(compute, "COMPUTE");

    // コンピュートシェーダー用のプログラムを作成
    ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(compute);
}

void Cshader::use() {
    glUseProgram(ID);
}

void Cshader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Cshader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Cshader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Cshader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Cshader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Cshader::setTexture(const std::string &name, int unit, GLuint texture) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(ID, name.c_str()), unit);
}

void Cshader::checkCompileErrors(GLuint cshader, std::string type) {
    int success;
    char infoLog[1024];
    if (type == "PROGRAM") {
        glGetProgramiv(cshader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(cshader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetShaderiv(cshader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(cshader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

void Cshader::dispatch(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {
    glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

void Cshader::waitForCompute() {
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}