#pragma once

#include <QWidget>
#include <QRect>

namespace zraw {

class CropOverlay : public QWidget {
    Q_OBJECT

public:
    explicit CropOverlay(QWidget* parent = nullptr);
    
    // Set crop region in normalized coordinates (0.0-1.0)
    void setCropRegion(float left, float top, float right, float bottom);
    void getCropRegion(float& left, float& top, float& right, float& bottom) const;
    
    // Reset to no crop
    void resetCrop();
    
    // Enable/disable crop mode
    void setCropMode(bool enabled);
    bool isCropMode() const { return m_cropMode; }
    
    // Set aspect ratio (0 = free, -1 = image aspect, others = width/height ratio)
    void setAspectRatio(float ratio);
    
    // Set the image aspect ratio (for "Original" mode)
    void setImageAspectRatio(float imageAspect);
    
    // Swap orientation (rotate aspect ratio 90 degrees)
    void setSwapOrientation(bool swap);
    
signals:
    void cropChanged(float left, float top, float right, float bottom);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private:
    enum class DragHandle {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Top,
        Bottom,
        Left,
        Right,
        Center
    };
    
    // Convert between normalized (0-1) and widget coordinates
    QRectF normalizedToWidget(float left, float top, float right, float bottom) const;
    void widgetToNormalized(const QRectF& rect, float& left, float& top, float& right, float& bottom) const;
    
    // Get handle at position
    DragHandle getHandleAt(const QPointF& pos) const;
    
    // Update crop rect while dragging
    void updateCropRect(const QPointF& pos);
    
    // Constrain to aspect ratio if set
    void constrainAspectRatio(QRectF& rect) const;
    
    bool m_cropMode;
    float m_cropLeft;
    float m_cropTop;
    float m_cropRight;
    float m_cropBottom;
    float m_aspectRatio;      // 0 = free, -1 = image aspect, >0 = specific ratio
    float m_imageAspectRatio; // Actual image aspect ratio
    bool m_swapOrientation;   // Swap width/height
    
    DragHandle m_dragHandle;
    QPointF m_dragStart;
    QRectF m_dragStartRect;
    
    static constexpr int HANDLE_SIZE = 12;
    static constexpr int HANDLE_MARGIN = 6;
};

} // namespace zraw
