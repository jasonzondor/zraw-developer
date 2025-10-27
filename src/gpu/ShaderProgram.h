#pragma once

#include <string>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <memory>

namespace zraw {

/**
 * Wrapper for OpenGL shader programs
 * Handles compilation and uniform management
 */
class ShaderProgram {
public:
    ShaderProgram();
    ~ShaderProgram();

    // Load and compile shaders
    bool loadVertexShader(const std::string& source);
    bool loadFragmentShader(const std::string& source);
    bool link();
    
    // Use shader
    void bind();
    void release();
    
    // Set uniforms
    void setUniform(const char* name, float value);
    void setUniform(const char* name, int value);
    void setUniform(const char* name, float x, float y);
    void setUniform(const char* name, float x, float y, float z);
    
    // Get program ID
    GLuint programId() const;
    
    // Error handling
    std::string lastError() const { return m_lastError; }

private:
    std::unique_ptr<QOpenGLShaderProgram> m_program;
    std::string m_lastError;
    
    void setError(const std::string& error);
};

} // namespace zraw
