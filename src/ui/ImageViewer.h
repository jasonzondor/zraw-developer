#pragma once

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QPointF>
#include "../gpu/GPUPipeline.h"
#include <memory>

namespace zraw {

/**
 * OpenGL widget for displaying processed images
 */
class ImageViewer : public QOpenGLWidget, protected QOpenGLExtraFunctions {
    Q_OBJECT

public:
    explicit ImageViewer(QWidget* parent = nullptr);
    ~ImageViewer();

    void setGPUPipeline(std::shared_ptr<GPUPipeline> pipeline);
    void updateDisplay();
    
    // View controls
    void resetView();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    
    // Mouse events for zoom and pan
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    std::shared_ptr<GPUPipeline> m_pipeline;
    GLuint m_displayVAO;
    GLuint m_displayVBO;
    GLuint m_displayShader;
    
    // View transform state
    float m_zoom;
    QPointF m_panOffset;
    
    // Mouse interaction state
    bool m_isPanning;
    QPointF m_lastMousePos;
    
    // Viewport state (for aspect ratio preservation)
    int m_viewportX;
    int m_viewportY;
    int m_viewportWidth;
    int m_viewportHeight;
    
    bool createDisplayShader();
    void renderTexture(GLuint texture);
    void updateTransform();
};

} // namespace zraw
