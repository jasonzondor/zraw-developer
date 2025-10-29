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
    void setTemperature(float temperature);
    void setTint(float tint);
    void setHighlights(float highlights);
    void setShadows(float shadows);
    void setVibrance(float vibrance);
    void setSaturation(float saturation);
    void setHighlightContrast(float highlightContrast);
    void setMidtoneContrast(float midtoneContrast);
    void setShadowContrast(float shadowContrast);
    void setWhites(float whites);
    void setBlacks(float blacks);
    
    // Output mode: 0=SDR, 1=HDR PQ, 2=HDR HLG, 3=Full ACES
    void setOutputMode(int mode);
    
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
    float m_temperature;
    float m_tint;
    float m_highlights;
    float m_shadows;
    float m_vibrance;
    float m_saturation;
    float m_highlightContrast;
    float m_midtoneContrast;
    float m_shadowContrast;
    float m_whites;
    float m_blacks;
    
    // Output mode (0=SDR, 1=HDR PQ, 2=HDR HLG, 3=Full ACES)
    int m_outputMode;
    
    // Vertex buffer for fullscreen quad
    GLuint m_vao;
    GLuint m_vbo;
    
    bool createShaders();
    bool createBuffers();
    void renderQuad();
};

} // namespace zraw
