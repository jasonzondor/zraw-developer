#include "CropOverlay.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

namespace zraw {

CropOverlay::CropOverlay(QWidget* parent)
    : QWidget(parent),
      m_cropMode(false),
      m_cropLeft(0.0f),
      m_cropTop(0.0f),
      m_cropRight(1.0f),
      m_cropBottom(1.0f),
      m_aspectRatio(0.0f),
      m_imageAspectRatio(1.0f),
      m_swapOrientation(false),
      m_dragHandle(DragHandle::None) {
    
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);
}

void CropOverlay::setCropRegion(float left, float top, float right, float bottom) {
    m_cropLeft = std::max(0.0f, std::min(1.0f, left));
    m_cropTop = std::max(0.0f, std::min(1.0f, top));
    m_cropRight = std::max(0.0f, std::min(1.0f, right));
    m_cropBottom = std::max(0.0f, std::min(1.0f, bottom));
    
    // Ensure right > left and bottom > top
    if (m_cropRight <= m_cropLeft) m_cropRight = m_cropLeft + 0.01f;
    if (m_cropBottom <= m_cropTop) m_cropBottom = m_cropTop + 0.01f;
    
    update();
}

void CropOverlay::getCropRegion(float& left, float& top, float& right, float& bottom) const {
    left = m_cropLeft;
    top = m_cropTop;
    right = m_cropRight;
    bottom = m_cropBottom;
}

void CropOverlay::resetCrop() {
    m_cropLeft = 0.0f;
    m_cropTop = 0.0f;
    m_cropRight = 1.0f;
    m_cropBottom = 1.0f;
    update();
    emit cropChanged(m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
}

void CropOverlay::setCropMode(bool enabled) {
    m_cropMode = enabled;
    setVisible(enabled);
    update();
}

void CropOverlay::setAspectRatio(float ratio) {
    m_aspectRatio = ratio;
    update();
}

void CropOverlay::setImageAspectRatio(float imageAspect) {
    m_imageAspectRatio = imageAspect;
    update();
}

void CropOverlay::setSwapOrientation(bool swap) {
    m_swapOrientation = swap;
    
    // Update the crop rectangle to apply the new orientation
    if (m_aspectRatio != 0.0f) {
        QRectF cropRect = normalizedToWidget(m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
        
        // Determine the target aspect ratio
        float targetRatio = m_aspectRatio;
        if (m_aspectRatio < 0.0f) {
            targetRatio = m_imageAspectRatio;
        }
        
        // Apply orientation swap
        if (m_swapOrientation && targetRatio > 0.0f) {
            targetRatio = 1.0f / targetRatio;
        }
        
        // Adjust the rectangle to match the new aspect ratio
        // Keep the center point fixed and adjust dimensions
        QPointF center = cropRect.center();
        float currentRatio = cropRect.width() / cropRect.height();
        
        if (std::abs(currentRatio - targetRatio) > 0.01f) {
            // Adjust based on which dimension should change
            if (targetRatio > currentRatio) {
                // Need wider - adjust width
                float newWidth = cropRect.height() * targetRatio;
                cropRect.setWidth(newWidth);
            } else {
                // Need taller - adjust height
                float newHeight = cropRect.width() / targetRatio;
                cropRect.setHeight(newHeight);
            }
            
            // Re-center
            cropRect.moveCenter(center);
            
            // Constrain to bounds
            if (cropRect.left() < 0) cropRect.moveLeft(0);
            if (cropRect.top() < 0) cropRect.moveTop(0);
            if (cropRect.right() > width()) cropRect.moveRight(width());
            if (cropRect.bottom() > height()) cropRect.moveBottom(height());
        }
        
        widgetToNormalized(cropRect, m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
        emit cropChanged(m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
    }
    
    update();
}

void CropOverlay::paintEvent(QPaintEvent* /*event*/) {
    if (!m_cropMode) return;
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Get crop rectangle in widget coordinates
    QRectF cropRect = normalizedToWidget(m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
    
    // Draw darkened overlay outside crop region
    QPainterPath fullPath;
    fullPath.addRect(rect());
    
    QPainterPath cropPath;
    cropPath.addRect(cropRect);
    
    QPainterPath overlayPath = fullPath.subtracted(cropPath);
    painter.fillPath(overlayPath, QColor(0, 0, 0, 128));
    
    // Draw crop rectangle border
    painter.setPen(QPen(QColor(255, 255, 255, 200), 2));
    painter.drawRect(cropRect);
    
    // Draw rule of thirds grid
    painter.setPen(QPen(QColor(255, 255, 255, 100), 1));
    float thirdWidth = cropRect.width() / 3.0f;
    float thirdHeight = cropRect.height() / 3.0f;
    
    // Vertical lines
    painter.drawLine(QPointF(cropRect.left() + thirdWidth, cropRect.top()),
                    QPointF(cropRect.left() + thirdWidth, cropRect.bottom()));
    painter.drawLine(QPointF(cropRect.left() + 2 * thirdWidth, cropRect.top()),
                    QPointF(cropRect.left() + 2 * thirdWidth, cropRect.bottom()));
    
    // Horizontal lines
    painter.drawLine(QPointF(cropRect.left(), cropRect.top() + thirdHeight),
                    QPointF(cropRect.right(), cropRect.top() + thirdHeight));
    painter.drawLine(QPointF(cropRect.left(), cropRect.top() + 2 * thirdHeight),
                    QPointF(cropRect.right(), cropRect.top() + 2 * thirdHeight));
    
    // Draw corner handles
    painter.setBrush(QColor(255, 255, 255, 255));
    painter.setPen(QPen(QColor(0, 0, 0, 200), 1));
    
    auto drawHandle = [&](const QPointF& center) {
        painter.drawRect(QRectF(center.x() - HANDLE_SIZE/2, center.y() - HANDLE_SIZE/2,
                               HANDLE_SIZE, HANDLE_SIZE));
    };
    
    // Corner handles
    drawHandle(cropRect.topLeft());
    drawHandle(cropRect.topRight());
    drawHandle(cropRect.bottomLeft());
    drawHandle(cropRect.bottomRight());
    
    // Edge handles
    drawHandle(QPointF(cropRect.center().x(), cropRect.top()));
    drawHandle(QPointF(cropRect.center().x(), cropRect.bottom()));
    drawHandle(QPointF(cropRect.left(), cropRect.center().y()));
    drawHandle(QPointF(cropRect.right(), cropRect.center().y()));
}

void CropOverlay::mousePressEvent(QMouseEvent* event) {
    if (!m_cropMode) {
        event->ignore();
        return;
    }
    
    m_dragHandle = getHandleAt(event->pos());
    m_dragStart = event->pos();
    m_dragStartRect = normalizedToWidget(m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
}

void CropOverlay::mouseMoveEvent(QMouseEvent* event) {
    if (!m_cropMode) {
        event->ignore();
        return;
    }
    
    if (m_dragHandle != DragHandle::None) {
        updateCropRect(event->pos());
    } else {
        // Update cursor based on handle
        DragHandle handle = getHandleAt(event->pos());
        switch (handle) {
            case DragHandle::TopLeft:
            case DragHandle::BottomRight:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case DragHandle::TopRight:
            case DragHandle::BottomLeft:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case DragHandle::Top:
            case DragHandle::Bottom:
                setCursor(Qt::SizeVerCursor);
                break;
            case DragHandle::Left:
            case DragHandle::Right:
                setCursor(Qt::SizeHorCursor);
                break;
            case DragHandle::Center:
                setCursor(Qt::SizeAllCursor);
                break;
            default:
                setCursor(Qt::ArrowCursor);
                break;
        }
    }
}

void CropOverlay::mouseReleaseEvent(QMouseEvent* event) {
    if (!m_cropMode) {
        event->ignore();
        return;
    }
    
    if (m_dragHandle != DragHandle::None) {
        m_dragHandle = DragHandle::None;
        emit cropChanged(m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
    }
}

void CropOverlay::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    update();
}

QRectF CropOverlay::normalizedToWidget(float left, float top, float right, float bottom) const {
    return QRectF(
        left * width(),
        top * height(),
        (right - left) * width(),
        (bottom - top) * height()
    );
}

void CropOverlay::widgetToNormalized(const QRectF& rect, float& left, float& top, float& right, float& bottom) const {
    left = rect.left() / width();
    top = rect.top() / height();
    right = rect.right() / width();
    bottom = rect.bottom() / height();
}

CropOverlay::DragHandle CropOverlay::getHandleAt(const QPointF& pos) const {
    QRectF cropRect = normalizedToWidget(m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
    
    auto isNear = [](const QPointF& p1, const QPointF& p2, float margin) {
        return (p1 - p2).manhattanLength() < margin;
    };
    
    // Check corner handles first
    if (isNear(pos, cropRect.topLeft(), HANDLE_SIZE + HANDLE_MARGIN))
        return DragHandle::TopLeft;
    if (isNear(pos, cropRect.topRight(), HANDLE_SIZE + HANDLE_MARGIN))
        return DragHandle::TopRight;
    if (isNear(pos, cropRect.bottomLeft(), HANDLE_SIZE + HANDLE_MARGIN))
        return DragHandle::BottomLeft;
    if (isNear(pos, cropRect.bottomRight(), HANDLE_SIZE + HANDLE_MARGIN))
        return DragHandle::BottomRight;
    
    // Check edge handles
    if (isNear(pos, QPointF(cropRect.center().x(), cropRect.top()), HANDLE_SIZE + HANDLE_MARGIN))
        return DragHandle::Top;
    if (isNear(pos, QPointF(cropRect.center().x(), cropRect.bottom()), HANDLE_SIZE + HANDLE_MARGIN))
        return DragHandle::Bottom;
    if (isNear(pos, QPointF(cropRect.left(), cropRect.center().y()), HANDLE_SIZE + HANDLE_MARGIN))
        return DragHandle::Left;
    if (isNear(pos, QPointF(cropRect.right(), cropRect.center().y()), HANDLE_SIZE + HANDLE_MARGIN))
        return DragHandle::Right;
    
    // Check if inside crop rect (for moving)
    if (cropRect.contains(pos))
        return DragHandle::Center;
    
    return DragHandle::None;
}

void CropOverlay::updateCropRect(const QPointF& pos) {
    QRectF newRect = m_dragStartRect;
    QPointF delta = pos - m_dragStart;
    
    switch (m_dragHandle) {
        case DragHandle::TopLeft:
            newRect.setTopLeft(m_dragStartRect.topLeft() + delta);
            break;
        case DragHandle::TopRight:
            newRect.setTopRight(m_dragStartRect.topRight() + delta);
            break;
        case DragHandle::BottomLeft:
            newRect.setBottomLeft(m_dragStartRect.bottomLeft() + delta);
            break;
        case DragHandle::BottomRight:
            newRect.setBottomRight(m_dragStartRect.bottomRight() + delta);
            break;
        case DragHandle::Top:
            newRect.setTop(m_dragStartRect.top() + delta.y());
            break;
        case DragHandle::Bottom:
            newRect.setBottom(m_dragStartRect.bottom() + delta.y());
            break;
        case DragHandle::Left:
            newRect.setLeft(m_dragStartRect.left() + delta.x());
            break;
        case DragHandle::Right:
            newRect.setRight(m_dragStartRect.right() + delta.x());
            break;
        case DragHandle::Center:
            newRect.translate(delta);
            break;
        default:
            return;
    }
    
    // Constrain to widget bounds
    newRect = newRect.intersected(rect());
    
    // Ensure minimum size
    if (newRect.width() < 50) newRect.setWidth(50);
    if (newRect.height() < 50) newRect.setHeight(50);
    
    // Apply aspect ratio constraint if set
    if (m_aspectRatio != 0.0f) {
        constrainAspectRatio(newRect);
    }
    
    // Convert back to normalized coordinates
    widgetToNormalized(newRect, m_cropLeft, m_cropTop, m_cropRight, m_cropBottom);
    
    update();
}

void CropOverlay::constrainAspectRatio(QRectF& rect) const {
    if (m_aspectRatio == 0.0f) return;  // Free aspect
    
    // Determine the target aspect ratio
    float targetRatio = m_aspectRatio;
    if (m_aspectRatio < 0.0f) {
        // Use image aspect ratio
        targetRatio = m_imageAspectRatio;
    }
    
    // Apply orientation swap if enabled
    if (m_swapOrientation && targetRatio > 0.0f) {
        targetRatio = 1.0f / targetRatio;
    }
    
    float currentRatio = rect.width() / rect.height();
    
    if (std::abs(currentRatio - targetRatio) < 0.01f) return;
    
    // Adjust based on which handle is being dragged
    switch (m_dragHandle) {
        case DragHandle::TopLeft:
        case DragHandle::TopRight:
        case DragHandle::BottomLeft:
        case DragHandle::BottomRight:
        case DragHandle::Left:
        case DragHandle::Right:
            // Adjust height to match width
            rect.setHeight(rect.width() / targetRatio);
            break;
        case DragHandle::Top:
        case DragHandle::Bottom:
            // Adjust width to match height
            rect.setWidth(rect.height() * targetRatio);
            break;
        default:
            break;
    }
}

} // namespace zraw
