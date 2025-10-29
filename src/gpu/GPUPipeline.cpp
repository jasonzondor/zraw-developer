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

// Fragment shader with color-science-correct adjustments
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

// ============================================================================
// COLOR SCIENCE FUNCTIONS
// ============================================================================

// Rec.2020 luminance weights (wider gamut, more accurate for modern displays)
float luminance(vec3 color) {
    return dot(color, vec3(0.2627, 0.6780, 0.0593));
}

// Linear to Log encoding (perceptually uniform for contrast adjustments)
vec3 linearToLog(vec3 color) {
    const float epsilon = 0.00001;
    return log2(max(color, epsilon)) / 10.0 + 0.5;
}

// Log to Linear decoding
vec3 logToLinear(vec3 logColor) {
    return exp2((logColor - 0.5) * 10.0);
}

// RGB to XYZ color space (D65 illuminant, sRGB primaries)
vec3 rgbToXYZ(vec3 rgb) {
    mat3 m = mat3(
        0.4124564, 0.3575761, 0.1804375,
        0.2126729, 0.7151522, 0.0721750,
        0.0193339, 0.1191920, 0.9503041
    );
    return m * rgb;
}

// XYZ to RGB color space
vec3 xyzToRGB(vec3 xyz) {
    mat3 m = mat3(
         3.2404542, -1.5371385, -0.4985314,
        -0.9692660,  1.8760108,  0.0415560,
         0.0556434, -0.2040259,  1.0572252
    );
    return m * xyz;
}

// XYZ to Lab color space (D65 illuminant)
vec3 xyzToLab(vec3 xyz) {
    // D65 reference white
    vec3 ref = vec3(0.95047, 1.0, 1.08883);
    xyz = xyz / ref;
    
    // Apply Lab transformation
    vec3 f;
    for (int i = 0; i < 3; i++) {
        if (xyz[i] > 0.008856) {
            f[i] = pow(xyz[i], 1.0/3.0);
        } else {
            f[i] = 7.787 * xyz[i] + 16.0/116.0;
        }
    }
    
    float L = 116.0 * f.y - 16.0;
    float a = 500.0 * (f.x - f.y);
    float b = 200.0 * (f.y - f.z);
    
    return vec3(L, a, b);
}

// Lab to XYZ color space
vec3 labToXYZ(vec3 lab) {
    float fy = (lab.x + 16.0) / 116.0;
    float fx = lab.y / 500.0 + fy;
    float fz = fy - lab.z / 200.0;
    
    vec3 xyz;
    xyz.x = (fx > 0.206897) ? pow(fx, 3.0) : (fx - 16.0/116.0) / 7.787;
    xyz.y = (fy > 0.206897) ? pow(fy, 3.0) : (fy - 16.0/116.0) / 7.787;
    xyz.z = (fz > 0.206897) ? pow(fz, 3.0) : (fz - 16.0/116.0) / 7.787;
    
    // D65 reference white
    vec3 ref = vec3(0.95047, 1.0, 1.08883);
    return xyz * ref;
}

// Lab to LCH (Lightness, Chroma, Hue) - perceptually uniform
vec3 labToLCH(vec3 lab) {
    float C = sqrt(lab.y * lab.y + lab.z * lab.z);
    float H = atan(lab.z, lab.y);
    return vec3(lab.x, C, H);
}

// LCH to Lab
vec3 lchToLab(vec3 lch) {
    float a = lch.y * cos(lch.z);
    float b = lch.y * sin(lch.z);
    return vec3(lch.x, a, b);
}

// Complete RGB to LCH pipeline (for saturation adjustments)
vec3 rgbToLCH(vec3 rgb) {
    return labToLCH(xyzToLab(rgbToXYZ(rgb)));
}

// Complete LCH to RGB pipeline
vec3 lchToRGB(vec3 lch) {
    return xyzToRGB(labToXYZ(lchToLab(lch)));
}

// Improved white balance using chromatic adaptation
vec3 applyWhiteBalance(vec3 color, float temp, float tint) {
    // Convert temperature from -100/+100 to multipliers
    // Positive temp = warmer (more red/yellow), negative = cooler (more blue)
    float tempFactor = temp / 100.0;
    float tintFactor = tint / 100.0;
    
    // Use Bradford chromatic adaptation matrix (industry standard)
    // This is a simplified version - full implementation would use CCT
    vec3 wb = color;
    
    // Temperature adjustment (red-blue axis)
    float warmth = 1.0 + tempFactor * 0.4;
    float coolness = 1.0 - tempFactor * 0.4;
    wb.r *= warmth;
    wb.b *= coolness;
    
    // Tint adjustment (green-magenta axis)
    wb.g *= 1.0 + tintFactor * 0.3;
    
    return wb;
}

void main() {
    // ========================================================================
    // PROPER COLOR SCIENCE PIPELINE ORDER
    // ========================================================================
    
    // 1. Sample input (linear RGB from RAW processing)
    vec3 color = texture(inputTexture, TexCoord).rgb;
    
    // 2. White Balance FIRST (in linear space, before exposure)
    //    This is correct because white balance is about the scene illuminant
    if (abs(temperature) > 0.1 || abs(tint) > 0.1) {
        color = applyWhiteBalance(color, temperature, tint);
    }
    
    // 3. Exposure (linear space multiplication)
    color *= pow(2.0, exposure);
    
    // 4. Highlights and Shadows Recovery (in linear space)
    //    Use improved luminance calculation and smooth falloffs
    if (abs(highlights) > 0.1 || abs(shadows) > 0.1) {
        float lum = luminance(color);
        
        // Gaussian-like falloff for smoother transitions
        float highlightMask = exp(-pow((1.0 - lum) / 0.3, 2.0));
        float shadowMask = exp(-pow(lum / 0.3, 2.0));
        
        // Apply recovery with better falloff
        float highlightFactor = 1.0 + (highlights / 100.0) * highlightMask * 0.8;
        float shadowFactor = 1.0 + (shadows / 100.0) * shadowMask * 0.8;
        
        color *= highlightFactor * shadowFactor;
    }
    
    // 5. Global Contrast (in LOG space for perceptual uniformity)
    if (abs(contrast) > 0.01) {
        vec3 logColor = linearToLog(color);
        // Apply contrast around middle gray (0.5 in log space = 18% gray)
        logColor = (logColor - 0.5) * (1.0 + contrast) + 0.5;
        color = logToLinear(logColor);
    }
    
    // 6. Local Contrast / Tone-specific contrast (in log space)
    if (abs(highlightContrast) > 0.1 || abs(midtoneContrast) > 0.1 || abs(shadowContrast) > 0.1) {
        float lum = luminance(color);
        
        // Better zone separation using smooth transitions
        float highlightMask = smoothstep(0.5, 0.9, lum);
        float shadowMask = smoothstep(0.5, 0.1, lum);
        float midtoneMask = 1.0 - highlightMask - shadowMask;
        
        // Apply in log space for better behavior
        vec3 logColor = linearToLog(color);
        float localContrast = 
            (highlightContrast / 100.0) * highlightMask +
            (midtoneContrast / 100.0) * midtoneMask +
            (shadowContrast / 100.0) * shadowMask;
        
        logColor = (logColor - 0.5) * (1.0 + localContrast) + 0.5;
        color = logToLinear(logColor);
    }
    
    // 7. Saturation and Vibrance (in LCH space - perceptually uniform, no hue shifts)
    if (abs(saturation) > 0.1 || abs(vibrance) > 0.1) {
        // Convert to LCH (Lightness, Chroma, Hue)
        vec3 lch = rgbToLCH(max(color, 0.0));
        
        // Saturation: adjust chroma directly
        if (abs(saturation) > 0.1) {
            lch.y *= 1.0 + (saturation / 100.0);
        }
        
        // Vibrance: non-linear chroma boost (affects low-chroma colors more)
        if (abs(vibrance) > 0.1) {
            float vibranceFactor = vibrance / 100.0;
            // Normalize chroma to 0-1 range (typical max chroma ~130)
            float chromaNorm = clamp(lch.y / 130.0, 0.0, 1.0);
            float vibranceMask = 1.0 - chromaNorm; // Boost muted colors more
            lch.y += vibranceFactor * vibranceMask * 50.0;
        }
        
        // Clamp chroma to reasonable range
        lch.y = max(lch.y, 0.0);
        
        // Convert back to RGB
        color = lchToRGB(lch);
    }
    
    // 8. Sharpening (should be near the end, in linear space)
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
        
        // Apply sharpening with luminance-based masking to avoid artifacts
        float lumMask = luminance(color);
        float sharpAmount = sharpness * (1.0 - smoothstep(0.9, 1.0, lumMask)); // Reduce in highlights
        color = color + (color - blur) * sharpAmount;
    }
    
    // 9. Final clamp to valid range
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
