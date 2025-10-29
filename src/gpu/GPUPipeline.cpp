#include "GPUPipeline.h"
#include <iostream>

namespace zraw {

// Vertex shader for fullscreen quad
static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

// Fragment shader with all adjustments
static const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform float exposure;
uniform float contrast;
uniform float sharpness;
uniform float temperature;
uniform float tint;
uniform float highlights;
uniform float shadows;
uniform float vibrance;
uniform float saturation;
uniform float highlightContrast;
uniform float midtoneContrast;
uniform float shadowContrast;
uniform vec2 texelSize;

// RGB to HSV conversion
vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// HSV to RGB conversion
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// Luminance calculation
float luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

void main() {
    // Sample center pixel
    vec3 color = texture(inputTexture, TexCoord).rgb;
    
    // Apply exposure (linear space)
    color *= pow(2.0, exposure);
    
    // Apply white balance (temperature and tint)
    if (abs(temperature) > 0.1 || abs(tint) > 0.1) {
        // Temperature: shift blue-yellow
        float tempFactor = temperature / 100.0;
        color.r *= 1.0 + tempFactor * 0.3;
        color.b *= 1.0 - tempFactor * 0.3;
        
        // Tint: shift green-magenta
        float tintFactor = tint / 100.0;
        color.g *= 1.0 + tintFactor * 0.3;
    }
    
    // Calculate luminance for tone-based adjustments
    float lum = luminance(color);
    
    // Highlights and shadows recovery
    if (abs(highlights) > 0.1 || abs(shadows) > 0.1) {
        float highlightMask = smoothstep(0.5, 1.0, lum);
        float shadowMask = smoothstep(0.5, 0.0, lum);
        
        float highlightFactor = 1.0 + (highlights / 100.0) * highlightMask;
        float shadowFactor = 1.0 + (shadows / 100.0) * shadowMask;
        
        color *= highlightFactor * shadowFactor;
    }
    
    // Local contrast (tone-specific contrast)
    if (abs(highlightContrast) > 0.1 || abs(midtoneContrast) > 0.1 || abs(shadowContrast) > 0.1) {
        float highlightMask = smoothstep(0.6, 1.0, lum);
        float shadowMask = smoothstep(0.4, 0.0, lum);
        float midtoneMask = 1.0 - highlightMask - shadowMask;
        
        float localContrast = 
            (highlightContrast / 100.0) * highlightMask +
            (midtoneContrast / 100.0) * midtoneMask +
            (shadowContrast / 100.0) * shadowMask;
        
        color = (color - lum) * (1.0 + localContrast) + lum;
    }
    
    // Apply sharpness using unsharp mask
    if (sharpness > 0.001) {
        vec3 blur = vec3(0.0);
        blur += texture(inputTexture, TexCoord + vec2(-1, -1) * texelSize).rgb;
        blur += texture(inputTexture, TexCoord + vec2( 0, -1) * texelSize).rgb * 2.0;
        blur += texture(inputTexture, TexCoord + vec2( 1, -1) * texelSize).rgb;
        blur += texture(inputTexture, TexCoord + vec2(-1,  0) * texelSize).rgb * 2.0;
        blur += texture(inputTexture, TexCoord + vec2( 1,  0) * texelSize).rgb * 2.0;
        blur += texture(inputTexture, TexCoord + vec2(-1,  1) * texelSize).rgb;
        blur += texture(inputTexture, TexCoord + vec2( 0,  1) * texelSize).rgb * 2.0;
        blur += texture(inputTexture, TexCoord + vec2( 1,  1) * texelSize).rgb;
        blur /= 12.0;
        
        vec3 sharp = color + (color - blur) * sharpness;
        color = sharp;
    }
    
    // Apply global contrast (around midpoint)
    color = (color - 0.5) * (1.0 + contrast) + 0.5;
    
    // Apply saturation and vibrance
    if (abs(saturation) > 0.1 || abs(vibrance) > 0.1) {
        vec3 hsv = rgb2hsv(color);
        
        // Saturation: linear adjustment
        if (abs(saturation) > 0.1) {
            hsv.y *= 1.0 + (saturation / 100.0);
        }
        
        // Vibrance: non-linear saturation boost (affects muted colors more)
        if (abs(vibrance) > 0.1) {
            float vibranceFactor = vibrance / 100.0;
            float satMask = 1.0 - hsv.y; // Boost less saturated colors more
            hsv.y += vibranceFactor * satMask * 0.5;
        }
        
        hsv.y = clamp(hsv.y, 0.0, 1.0);
        color = hsv2rgb(hsv);
    }
    
    // Clamp to valid range
    color = clamp(color, 0.0, 1.0);
    
    FragColor = vec4(color, 1.0);
}
)";

GPUPipeline::GPUPipeline()
    : m_context(std::make_unique<GLContext>()),
      m_shader(std::make_unique<ShaderProgram>()),
      m_width(0), m_height(0),
      m_exposure(0.0f), m_contrast(0.0f), m_sharpness(0.0f),
      m_temperature(0.0f), m_tint(0.0f),
      m_highlights(0.0f), m_shadows(0.0f),
      m_vibrance(0.0f), m_saturation(0.0f),
      m_highlightContrast(0.0f), m_midtoneContrast(0.0f), m_shadowContrast(0.0f),
      m_vao(0), m_vbo(0) {
}

GPUPipeline::~GPUPipeline() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

bool GPUPipeline::initialize() {
    initializeOpenGLFunctions();
    
    if (!m_context->initialize()) {
        return false;
    }
    
    if (!createShaders()) {
        return false;
    }
    
    if (!createBuffers()) {
        return false;
    }
    
    return true;
}

bool GPUPipeline::createShaders() {
    if (!m_shader->loadVertexShader(vertexShaderSource)) {
        std::cerr << m_shader->lastError() << std::endl;
        return false;
    }
    
    if (!m_shader->loadFragmentShader(fragmentShaderSource)) {
        std::cerr << m_shader->lastError() << std::endl;
        return false;
    }
    
    if (!m_shader->link()) {
        std::cerr << m_shader->lastError() << std::endl;
        return false;
    }
    
    return true;
}

bool GPUPipeline::createBuffers() {
    // Fullscreen quad vertices
    float vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // TexCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    return true;
}

bool GPUPipeline::uploadImage(std::shared_ptr<ImageBuffer> buffer) {
    if (!buffer || buffer->width() == 0 || buffer->height() == 0) {
        std::cerr << "Invalid image buffer" << std::endl;
        return false;
    }
    
    m_width = buffer->width();
    m_height = buffer->height();
    
    // Create texture
    m_inputTexture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
    m_inputTexture->setFormat(QOpenGLTexture::RGB16_UNorm);
    m_inputTexture->setSize(m_width, m_height);
    m_inputTexture->allocateStorage();
    m_inputTexture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt16, buffer->data());
    m_inputTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_inputTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_inputTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
    
    // Create framebuffer for output
    QOpenGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(GL_RGB16);
    m_fbo = std::make_unique<QOpenGLFramebufferObject>(m_width, m_height, format);
    
    return true;
}

void GPUPipeline::setExposure(float exposure) {
    m_exposure = exposure;
}

void GPUPipeline::setContrast(float contrast) {
    m_contrast = contrast;
}

void GPUPipeline::setSharpness(float sharpness) {
    m_sharpness = sharpness;
}

void GPUPipeline::setTemperature(float temperature) {
    m_temperature = temperature;
}

void GPUPipeline::setTint(float tint) {
    m_tint = tint;
}

void GPUPipeline::setHighlights(float highlights) {
    m_highlights = highlights;
}

void GPUPipeline::setShadows(float shadows) {
    m_shadows = shadows;
}

void GPUPipeline::setVibrance(float vibrance) {
    m_vibrance = vibrance;
}

void GPUPipeline::setSaturation(float saturation) {
    m_saturation = saturation;
}

void GPUPipeline::setHighlightContrast(float highlightContrast) {
    m_highlightContrast = highlightContrast;
}

void GPUPipeline::setMidtoneContrast(float midtoneContrast) {
    m_midtoneContrast = midtoneContrast;
}

void GPUPipeline::setShadowContrast(float shadowContrast) {
    m_shadowContrast = shadowContrast;
}

bool GPUPipeline::process() {
    if (!m_inputTexture || !m_fbo) {
        std::cerr << "Pipeline not ready for processing" << std::endl;
        return false;
    }
    
    // Bind framebuffer
    m_fbo->bind();
    
    // Set viewport
    glViewport(0, 0, m_width, m_height);
    
    // Clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use shader
    m_shader->bind();
    
    // Set uniforms
    m_shader->setUniform("inputTexture", 0);
    m_shader->setUniform("exposure", m_exposure);
    m_shader->setUniform("contrast", m_contrast);
    m_shader->setUniform("sharpness", m_sharpness);
    m_shader->setUniform("temperature", m_temperature);
    m_shader->setUniform("tint", m_tint);
    m_shader->setUniform("highlights", m_highlights);
    m_shader->setUniform("shadows", m_shadows);
    m_shader->setUniform("vibrance", m_vibrance);
    m_shader->setUniform("saturation", m_saturation);
    m_shader->setUniform("highlightContrast", m_highlightContrast);
    m_shader->setUniform("midtoneContrast", m_midtoneContrast);
    m_shader->setUniform("shadowContrast", m_shadowContrast);
    m_shader->setUniform("texelSize", 1.0f / m_width, 1.0f / m_height);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    m_inputTexture->bind();
    
    // Render quad
    renderQuad();
    
    // Cleanup
    m_inputTexture->release();
    m_shader->release();
    m_fbo->release();
    
    return true;
}

void GPUPipeline::renderQuad() {
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

std::shared_ptr<ImageBuffer> GPUPipeline::downloadImage() {
    if (!m_fbo) {
        return nullptr;
    }
    
    auto buffer = std::make_shared<ImageBuffer>(m_width, m_height, 3);
    
    m_fbo->bind();
    glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_SHORT, buffer->data());
    m_fbo->release();
    
    return buffer;
}

GLuint GPUPipeline::getOutputTexture() const {
    return m_fbo ? m_fbo->texture() : 0;
}

} // namespace zraw
