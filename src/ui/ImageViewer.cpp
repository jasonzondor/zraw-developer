#include "ImageViewer.h"
#include <QOpenGLShaderProgram>
#include <QWheelEvent>
#include <QMouseEvent>
#include <iostream>

namespace zraw {

// Shaders with zoom and pan support
static const char* displayVertexShader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;

uniform float zoom;
uniform vec2 panOffset;

void main() {
    // Pass through position (fills viewport)
    gl_Position = vec4(aPos, 0.0, 1.0);
    
    // Apply zoom and pan to texture coordinates
    vec2 tc = aTexCoord;
    
    // Center around 0.5
    tc -= 0.5;
    
    // Apply zoom (divide to zoom in on texture)
    tc /= zoom;
    
    // Apply pan
    tc -= panOffset * 0.5;
    
    // Move back to 0-1 range
    tc += 0.5;
    
    // Flip Y coordinate to fix upside-down image
    tc.y = 1.0 - tc.y;
    
    TexCoord = tc;
}
)";

static const char* displayFragmentShader = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D displayTexture;

void main() {
    // Discard pixels outside valid texture range when zoomed out
    if (TexCoord.x < 0.0 || TexCoord.x > 1.0 || 
        TexCoord.y < 0.0 || TexCoord.y > 1.0) {
        discard;
    }
    FragColor = texture(displayTexture, TexCoord);
}
)";

ImageViewer::ImageViewer(QWidget* parent)
    : QOpenGLWidget(parent),
      m_displayVAO(0), m_displayVBO(0), m_displayShader(0),
      m_zoom(1.0f), m_panOffset(0.0f, 0.0f),
      m_isPanning(false),
      m_viewportX(0), m_viewportY(0), m_viewportWidth(0), m_viewportHeight(0) {
    setMouseTracking(true);
}

ImageViewer::~ImageViewer() {
    makeCurrent();
    if (m_displayVAO) glDeleteVertexArrays(1, &m_displayVAO);
    if (m_displayVBO) glDeleteBuffers(1, &m_displayVBO);
    if (m_displayShader) glDeleteProgram(m_displayShader);
    doneCurrent();
}

void ImageViewer::setGPUPipeline(std::shared_ptr<GPUPipeline> pipeline) {
    m_pipeline = pipeline;
    // Trigger resize to recalculate aspect ratio
    if (m_pipeline) {
        resizeGL(width(), height());
        update();
    }
}

void ImageViewer::updateDisplay() {
    update();
}

void ImageViewer::initializeGL() {
    initializeOpenGLFunctions();
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    createDisplayShader();
    
    // Create fullscreen quad
    float vertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_displayVAO);
    glGenBuffers(1, &m_displayVBO);
    
    glBindVertexArray(m_displayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_displayVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void ImageViewer::paintGL() {
    qreal dpr = devicePixelRatio();
    
    // Clear the entire window first (for letterboxing) - use physical pixels
    glViewport(0, 0, width() * dpr, height() * dpr);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (m_pipeline) {
        GLuint texture = m_pipeline->getOutputTexture();
        if (texture) {
            // Use the stored viewport dimensions scaled by device pixel ratio
            glViewport(m_viewportX * dpr, m_viewportY * dpr, 
                      m_viewportWidth * dpr, m_viewportHeight * dpr);
            
            renderTexture(texture);
        }
    }
}

void ImageViewer::resizeGL(int w, int h) {
    if (w == 0 || h == 0) {
        m_viewportX = 0;
        m_viewportY = 0;
        m_viewportWidth = w;
        m_viewportHeight = h;
        return;
    }
    
    if (!m_pipeline || m_pipeline->width() == 0 || m_pipeline->height() == 0) {
        m_viewportX = 0;
        m_viewportY = 0;
        m_viewportWidth = w;
        m_viewportHeight = h;
        return;
    }
    
    // Calculate aspect ratios
    float imageAspect = static_cast<float>(m_pipeline->width()) / static_cast<float>(m_pipeline->height());
    float windowAspect = static_cast<float>(w) / static_cast<float>(h);
    
    m_viewportX = 0;
    m_viewportY = 0;
    m_viewportWidth = w;
    m_viewportHeight = h;
    
    // Fit image to window while maintaining aspect ratio
    if (windowAspect > imageAspect) {
        // Window is wider than image - add letterboxing on sides
        m_viewportWidth = static_cast<int>(static_cast<float>(h) * imageAspect);
        m_viewportX = (w - m_viewportWidth) / 2;
    } else {
        // Window is taller than image - add letterboxing on top/bottom
        m_viewportHeight = static_cast<int>(static_cast<float>(w) / imageAspect);
        m_viewportY = (h - m_viewportHeight) / 2;
    }
}

bool ImageViewer::createDisplayShader() {
    // Compile vertex shader
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &displayVertexShader, nullptr);
    glCompileShader(vertShader);
    
    // Check compilation
    GLint success;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        return false;
    }
    
    // Compile fragment shader
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &displayFragmentShader, nullptr);
    glCompileShader(fragShader);
    
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        return false;
    }
    
    // Link program
    m_displayShader = glCreateProgram();
    glAttachShader(m_displayShader, vertShader);
    glAttachShader(m_displayShader, fragShader);
    glLinkProgram(m_displayShader);
    
    glGetProgramiv(m_displayShader, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_displayShader, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return false;
    }
    
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    return true;
}

void ImageViewer::renderTexture(GLuint texture) {
    glUseProgram(m_displayShader);
    
    // Set zoom and pan uniforms
    glUniform1f(glGetUniformLocation(m_displayShader, "zoom"), m_zoom);
    glUniform2f(glGetUniformLocation(m_displayShader, "panOffset"), 
                m_panOffset.x(), m_panOffset.y());
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(m_displayShader, "displayTexture"), 0);
    
    glBindVertexArray(m_displayVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glUseProgram(0);
}

void ImageViewer::resetView() {
    m_zoom = 1.0f;
    m_panOffset = QPointF(0.0f, 0.0f);
    update();
}

void ImageViewer::wheelEvent(QWheelEvent* event) {
    // Zoom with mouse wheel
    float zoomFactor = 1.1f;
    
    if (event->angleDelta().y() > 0) {
        // Zoom in
        m_zoom *= zoomFactor;
    } else {
        // Zoom out
        m_zoom /= zoomFactor;
    }
    
    // Clamp zoom range
    m_zoom = std::max(0.1f, std::min(m_zoom, 10.0f));
    
    update();
    event->accept();
}

void ImageViewer::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_lastMousePos = event->position();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}

void ImageViewer::mouseMoveEvent(QMouseEvent* event) {
    if (m_isPanning) {
        QPointF delta = event->position() - m_lastMousePos;
        
        // Convert pixel delta to normalized coordinates
        float dx = (delta.x() / width()) * 2.0f;
        float dy = -(delta.y() / height()) * 2.0f; // Flip Y
        
        m_panOffset += QPointF(dx, dy);
        m_lastMousePos = event->position();
        
        update();
        event->accept();
    }
}

void ImageViewer::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    }
}

void ImageViewer::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        resetView();
        event->accept();
    }
}

} // namespace zraw
