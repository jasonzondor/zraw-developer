#include "AdjustmentPanel.h"
#include <QGroupBox>
#include <QHBoxLayout>

namespace zraw {

AdjustmentPanel::AdjustmentPanel(QWidget* parent)
    : QWidget(parent) {
    createUI();
}

AdjustmentPanel::~AdjustmentPanel() {
}

void AdjustmentPanel::createUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // Exposure slider (-3.0 to +3.0, step 0.01)
    auto* exposureGroup = createSliderGroup("Exposure", &m_exposureSlider, &m_exposureLabel,
                                            -300, 300, 0);
    layout->addWidget(exposureGroup);
    
    connect(m_exposureSlider, &QSlider::valueChanged, this, [this](int value) {
        updateExposureLabel(value);
        emit exposureChanged(value / 100.0f);
    });
    
    // Contrast slider (-1.0 to +1.0, step 0.01)
    auto* contrastGroup = createSliderGroup("Contrast", &m_contrastSlider, &m_contrastLabel,
                                            -100, 100, 0);
    layout->addWidget(contrastGroup);
    
    connect(m_contrastSlider, &QSlider::valueChanged, this, [this](int value) {
        updateContrastLabel(value);
        emit contrastChanged(value / 100.0f);
    });
    
    // Sharpness slider (0.0 to 2.0, step 0.01)
    auto* sharpnessGroup = createSliderGroup("Sharpness", &m_sharpnessSlider, &m_sharpnessLabel,
                                             0, 200, 0);
    layout->addWidget(sharpnessGroup);
    
    connect(m_sharpnessSlider, &QSlider::valueChanged, this, [this](int value) {
        updateSharpnessLabel(value);
        emit sharpnessChanged(value / 100.0f);
    });
    
    layout->addStretch();
    
    // Initialize labels
    updateExposureLabel(0);
    updateContrastLabel(0);
    updateSharpnessLabel(0);
}

QWidget* AdjustmentPanel::createSliderGroup(const QString& name, QSlider** slider,
                                            QLabel** valueLabel, int min, int max, int defaultVal) {
    auto* group = new QGroupBox(name);
    auto* layout = new QVBoxLayout(group);
    
    // Value label
    *valueLabel = new QLabel("0.00");
    (*valueLabel)->setAlignment(Qt::AlignCenter);
    layout->addWidget(*valueLabel);
    
    // Slider
    *slider = new QSlider(Qt::Horizontal);
    (*slider)->setMinimum(min);
    (*slider)->setMaximum(max);
    (*slider)->setValue(defaultVal);
    (*slider)->setTickPosition(QSlider::TicksBelow);
    (*slider)->setTickInterval((max - min) / 10);
    layout->addWidget(*slider);
    
    return group;
}

void AdjustmentPanel::updateExposureLabel(int value) {
    float fValue = value / 100.0f;
    m_exposureLabel->setText(QString::number(fValue, 'f', 2) + " EV");
}

void AdjustmentPanel::updateContrastLabel(int value) {
    float fValue = value / 100.0f;
    m_contrastLabel->setText(QString::number(fValue, 'f', 2));
}

void AdjustmentPanel::updateSharpnessLabel(int value) {
    float fValue = value / 100.0f;
    m_sharpnessLabel->setText(QString::number(fValue, 'f', 2));
}

// Getters
float AdjustmentPanel::exposure() const {
    return m_exposureSlider->value() / 100.0f;
}

float AdjustmentPanel::contrast() const {
    return m_contrastSlider->value() / 100.0f;
}

float AdjustmentPanel::sharpness() const {
    return m_sharpnessSlider->value() / 100.0f;
}

// Setters (block signals to prevent triggering changes)
void AdjustmentPanel::setExposure(float value) {
    m_exposureSlider->blockSignals(true);
    m_exposureSlider->setValue(static_cast<int>(value * 100));
    updateExposureLabel(m_exposureSlider->value());
    m_exposureSlider->blockSignals(false);
}

void AdjustmentPanel::setContrast(float value) {
    m_contrastSlider->blockSignals(true);
    m_contrastSlider->setValue(static_cast<int>(value * 100));
    updateContrastLabel(m_contrastSlider->value());
    m_contrastSlider->blockSignals(false);
}

void AdjustmentPanel::setSharpness(float value) {
    m_sharpnessSlider->blockSignals(true);
    m_sharpnessSlider->setValue(static_cast<int>(value * 100));
    updateSharpnessLabel(m_sharpnessSlider->value());
    m_sharpnessSlider->blockSignals(false);
}

} // namespace zraw
