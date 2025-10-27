#include "GLContext.h"
#include <iostream>

namespace zraw {

GLContext::GLContext()
    : m_initialized(false) {
}

GLContext::~GLContext() {
}

bool GLContext::initialize() {
    initializeOpenGLFunctions();
    
    m_initialized = true;
    
    std::cout << "OpenGL initialized:" << std::endl;
    std::cout << "  Version: " << version() << std::endl;
    std::cout << "  Vendor: " << vendor() << std::endl;
    std::cout << "  Renderer: " << renderer() << std::endl;
    
    return true;
}

std::string GLContext::version() const {
    const char* ver = reinterpret_cast<const char*>(const_cast<GLContext*>(this)->glGetString(GL_VERSION));
    return ver ? std::string(ver) : "Unknown";
}

std::string GLContext::vendor() const {
    const char* vnd = reinterpret_cast<const char*>(const_cast<GLContext*>(this)->glGetString(GL_VENDOR));
    return vnd ? std::string(vnd) : "Unknown";
}

std::string GLContext::renderer() const {
    const char* rnd = reinterpret_cast<const char*>(const_cast<GLContext*>(this)->glGetString(GL_RENDERER));
    return rnd ? std::string(rnd) : "Unknown";
}

} // namespace zraw
