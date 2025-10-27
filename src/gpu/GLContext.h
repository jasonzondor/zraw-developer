#pragma once

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <memory>

namespace zraw {

/**
 * OpenGL context manager
 * Handles OpenGL initialization and state
 */
class GLContext : protected QOpenGLFunctions {
public:
    GLContext();
    ~GLContext();

    // Initialize OpenGL
    bool initialize();
    
    // Check if initialized
    bool isInitialized() const { return m_initialized; }
    
    // Get OpenGL version
    std::string version() const;
    std::string vendor() const;
    std::string renderer() const;

private:
    bool m_initialized;
};

} // namespace zraw
