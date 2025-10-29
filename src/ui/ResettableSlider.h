#pragma once

#include <QSlider>
#include <QMouseEvent>

namespace zraw {

/**
 * A QSlider that resets to its default value on double-click
 */
class ResettableSlider : public QSlider {
    Q_OBJECT

public:
    explicit ResettableSlider(Qt::Orientation orientation, QWidget* parent = nullptr)
        : QSlider(orientation, parent), m_defaultValue(0) {}
    
    void setDefaultValue(int value) {
        m_defaultValue = value;
    }
    
    int defaultValue() const {
        return m_defaultValue;
    }

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            setValue(m_defaultValue);
            event->accept();
        } else {
            QSlider::mouseDoubleClickEvent(event);
        }
    }

private:
    int m_defaultValue;
};

} // namespace zraw
