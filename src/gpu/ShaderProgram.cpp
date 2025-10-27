#include "ShaderProgram.h"
#include <iostream>

namespace zraw {

ShaderProgram::ShaderProgram()
    : m_program(std::make_unique<QOpenGLShaderProgram>()) {
}

ShaderProgram::~ShaderProgram() {
}

bool ShaderProgram::loadVertexShader(const std::string& source) {
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, source.c_str())) {
        setError("Failed to compile vertex shader: " + m_program->log().toStdString());
        return false;
    }
    return true;
}

bool ShaderProgram::loadFragmentShader(const std::string& source) {
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, source.c_str())) {
        setError("Failed to compile fragment shader: " + m_program->log().toStdString());
        return false;
    }
    return true;
}

bool ShaderProgram::link() {
    if (!m_program->link()) {
        setError("Failed to link shader program: " + m_program->log().toStdString());
        return false;
    }
    return true;
}

void ShaderProgram::bind() {
    m_program->bind();
}

void ShaderProgram::release() {
    m_program->release();
}

void ShaderProgram::setUniform(const char* name, float value) {
    m_program->setUniformValue(name, value);
}

void ShaderProgram::setUniform(const char* name, int value) {
    m_program->setUniformValue(name, value);
}

void ShaderProgram::setUniform(const char* name, float x, float y) {
    m_program->setUniformValue(name, x, y);
}

void ShaderProgram::setUniform(const char* name, float x, float y, float z) {
    m_program->setUniformValue(name, x, y, z);
}

GLuint ShaderProgram::programId() const {
    return m_program->programId();
}

void ShaderProgram::setError(const std::string& error) {
    m_lastError = error;
    std::cerr << "ShaderProgram error: " << error << std::endl;
}

} // namespace zraw
