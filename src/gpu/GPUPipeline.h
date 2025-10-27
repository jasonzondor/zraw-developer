#pragma once

#include "ShaderProgram.h"
#include "GLContext.h"
#include "../core/ImageBuffer.h"
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QOpenGLExtraFunctions>
#include <memory>
#include <map>

namespace zraw {

/**
 * GPU processing pipeline
 * Manages textures, framebuffers, and shader execution
 */
class GPUPipeline : protected QOpenGLExtraFunctions {
public:
    GPUPipeline();
    ~GPUPipeline();

    // Initialize pipeline
    bool initialize();
    
    // Check if initialized
    bool isInitialized() const { return m_context && m_context->isInitialized(); }
    
    // Upload image to GPU
    bool uploadImage(std::shared_ptr<ImageBuffer> buffer);
    
    // Apply adjustments
    void setExposure(float exposure);
    void setContrast(float contrast);
    void setSharpness(float sharpness);
    
    // Process image with current settings
    bool process();
    
    // Download processed image from GPU
    std::shared_ptr<ImageBuffer> downloadImage();
    
    // Get texture for rendering
    GLuint getOutputTexture() const;
    
    // Get image dimensions
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    std::unique_ptr<GLContext> m_context;
    std::unique_ptr<ShaderProgram> m_shader;
    std::unique_ptr<QOpenGLTexture> m_inputTexture;
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo;
    
    int m_width;
    int m_height;
    
    // Adjustment parameters
    float m_exposure;
    float m_contrast;
    float m_sharpness;
    
    // Vertex buffer for fullscreen quad
    GLuint m_vao;
    GLuint m_vbo;
    
    bool createShaders();
    bool createBuffers();
    void renderQuad();
};

} // namespace zraw
