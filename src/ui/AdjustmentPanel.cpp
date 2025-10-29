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
    
    // White Balance - Temperature slider (-100 to +100, step 1)
    auto* temperatureGroup = createSliderGroup("Temperature", &m_temperatureSlider, &m_temperatureLabel,
                                               -100, 100, 0);
    layout->addWidget(temperatureGroup);
    
    connect(m_temperatureSlider, &QSlider::valueChanged, this, [this](int value) {
        updateTemperatureLabel(value);
        emit temperatureChanged(static_cast<float>(value));
    });
    
    // White Balance - Tint slider (-100 to +100, step 1)
    auto* tintGroup = createSliderGroup("Tint", &m_tintSlider, &m_tintLabel,
                                        -100, 100, 0);
    layout->addWidget(tintGroup);
    
    connect(m_tintSlider, &QSlider::valueChanged, this, [this](int value) {
        updateTintLabel(value);
        emit tintChanged(static_cast<float>(value));
    });
    
    // Highlights slider (-100 to +100, step 1)
    auto* highlightsGroup = createSliderGroup("Highlights", &m_highlightsSlider, &m_highlightsLabel,
                                              -100, 100, 0);
    layout->addWidget(highlightsGroup);
    
    connect(m_highlightsSlider, &QSlider::valueChanged, this, [this](int value) {
        updateHighlightsLabel(value);
        emit highlightsChanged(static_cast<float>(value));
    });
    
    // Shadows slider (-100 to +100, step 1)
    auto* shadowsGroup = createSliderGroup("Shadows", &m_shadowsSlider, &m_shadowsLabel,
                                           -100, 100, 0);
    layout->addWidget(shadowsGroup);
    
    connect(m_shadowsSlider, &QSlider::valueChanged, this, [this](int value) {
        updateShadowsLabel(value);
        emit shadowsChanged(static_cast<float>(value));
    });
    
    // Vibrance slider (-100 to +100, step 1)
    auto* vibranceGroup = createSliderGroup("Vibrance", &m_vibranceSlider, &m_vibranceLabel,
                                            -100, 100, 0);
    layout->addWidget(vibranceGroup);
    
    connect(m_vibranceSlider, &QSlider::valueChanged, this, [this](int value) {
        updateVibranceLabel(value);
        emit vibranceChanged(static_cast<float>(value));
    });
    
    // Saturation slider (-100 to +100, step 1)
    auto* saturationGroup = createSliderGroup("Saturation", &m_saturationSlider, &m_saturationLabel,
                                              -100, 100, 0);
    layout->addWidget(saturationGroup);
    
    connect(m_saturationSlider, &QSlider::valueChanged, this, [this](int value) {
        updateSaturationLabel(value);
        emit saturationChanged(static_cast<float>(value));
    });
    
    // Local Contrast - Highlights (-100 to +100, step 1)
    auto* highlightContrastGroup = createSliderGroup("Highlight Contrast", &m_highlightContrastSlider, 
                                                     &m_highlightContrastLabel, -100, 100, 0);
    layout->addWidget(highlightContrastGroup);
    
    connect(m_highlightContrastSlider, &QSlider::valueChanged, this, [this](int value) {
        updateHighlightContrastLabel(value);
        emit highlightContrastChanged(static_cast<float>(value));
    });
    
    // Local Contrast - Midtones (-100 to +100, step 1)
    auto* midtoneContrastGroup = createSliderGroup("Midtone Contrast", &m_midtoneContrastSlider, 
                                                   &m_midtoneContrastLabel, -100, 100, 0);
    layout->addWidget(midtoneContrastGroup);
    
    connect(m_midtoneContrastSlider, &QSlider::valueChanged, this, [this](int value) {
        updateMidtoneContrastLabel(value);
        emit midtoneContrastChanged(static_cast<float>(value));
    });
    
    // Local Contrast - Shadows (-100 to +100, step 1)
    auto* shadowContrastGroup = createSliderGroup("Shadow Contrast", &m_shadowContrastSlider, 
                                                  &m_shadowContrastLabel, -100, 100, 0);
    layout->addWidget(shadowContrastGroup);
    
    connect(m_shadowContrastSlider, &QSlider::valueChanged, this, [this](int value) {
        updateShadowContrastLabel(value);
        emit shadowContrastChanged(static_cast<float>(value));
    });
    
    layout->addStretch();
    
    // Initialize labels
    updateExposureLabel(0);
    updateContrastLabel(0);
    updateSharpnessLabel(0);
    updateTemperatureLabel(0);
    updateTintLabel(0);
    updateHighlightsLabel(0);
    updateShadowsLabel(0);
    updateVibranceLabel(0);
    updateSaturationLabel(0);
    updateHighlightContrastLabel(0);
    updateMidtoneContrastLabel(0);
    updateShadowContrastLabel(0);
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

void AdjustmentPanel::updateTemperatureLabel(int value) {
    m_temperatureLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateTintLabel(int value) {
    m_tintLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateHighlightsLabel(int value) {
    m_highlightsLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateShadowsLabel(int value) {
    m_shadowsLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateVibranceLabel(int value) {
    m_vibranceLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateSaturationLabel(int value) {
    m_saturationLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateHighlightContrastLabel(int value) {
    m_highlightContrastLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateMidtoneContrastLabel(int value) {
    m_midtoneContrastLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateShadowContrastLabel(int value) {
    m_shadowContrastLabel->setText(QString::number(value));
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

float AdjustmentPanel::temperature() const {
    return static_cast<float>(m_temperatureSlider->value());
}

float AdjustmentPanel::tint() const {
    return static_cast<float>(m_tintSlider->value());
}

float AdjustmentPanel::highlights() const {
    return static_cast<float>(m_highlightsSlider->value());
}

float AdjustmentPanel::shadows() const {
    return static_cast<float>(m_shadowsSlider->value());
}

float AdjustmentPanel::vibrance() const {
    return static_cast<float>(m_vibranceSlider->value());
}

float AdjustmentPanel::saturation() const {
    return static_cast<float>(m_saturationSlider->value());
}

float AdjustmentPanel::highlightContrast() const {
    return static_cast<float>(m_highlightContrastSlider->value());
}

float AdjustmentPanel::midtoneContrast() const {
    return static_cast<float>(m_midtoneContrastSlider->value());
}

float AdjustmentPanel::shadowContrast() const {
    return static_cast<float>(m_shadowContrastSlider->value());
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

void AdjustmentPanel::setTemperature(float value) {
    m_temperatureSlider->blockSignals(true);
    m_temperatureSlider->setValue(static_cast<int>(value));
    updateTemperatureLabel(m_temperatureSlider->value());
    m_temperatureSlider->blockSignals(false);
}

void AdjustmentPanel::setTint(float value) {
    m_tintSlider->blockSignals(true);
    m_tintSlider->setValue(static_cast<int>(value));
    updateTintLabel(m_tintSlider->value());
    m_tintSlider->blockSignals(false);
}

void AdjustmentPanel::setHighlights(float value) {
    m_highlightsSlider->blockSignals(true);
    m_highlightsSlider->setValue(static_cast<int>(value));
    updateHighlightsLabel(m_highlightsSlider->value());
    m_highlightsSlider->blockSignals(false);
}

void AdjustmentPanel::setShadows(float value) {
    m_shadowsSlider->blockSignals(true);
    m_shadowsSlider->setValue(static_cast<int>(value));
    updateShadowsLabel(m_shadowsSlider->value());
    m_shadowsSlider->blockSignals(false);
}

void AdjustmentPanel::setVibrance(float value) {
    m_vibranceSlider->blockSignals(true);
    m_vibranceSlider->setValue(static_cast<int>(value));
    updateVibranceLabel(m_vibranceSlider->value());
    m_vibranceSlider->blockSignals(false);
}

void AdjustmentPanel::setSaturation(float value) {
    m_saturationSlider->blockSignals(true);
    m_saturationSlider->setValue(static_cast<int>(value));
    updateSaturationLabel(m_saturationSlider->value());
    m_saturationSlider->blockSignals(false);
}

void AdjustmentPanel::setHighlightContrast(float value) {
    m_highlightContrastSlider->blockSignals(true);
    m_highlightContrastSlider->setValue(static_cast<int>(value));
    updateHighlightContrastLabel(m_highlightContrastSlider->value());
    m_highlightContrastSlider->blockSignals(false);
}

void AdjustmentPanel::setMidtoneContrast(float value) {
    m_midtoneContrastSlider->blockSignals(true);
    m_midtoneContrastSlider->setValue(static_cast<int>(value));
    updateMidtoneContrastLabel(m_midtoneContrastSlider->value());
    m_midtoneContrastSlider->blockSignals(false);
}

void AdjustmentPanel::setShadowContrast(float value) {
    m_shadowContrastSlider->blockSignals(true);
    m_shadowContrastSlider->setValue(static_cast<int>(value));
    updateShadowContrastLabel(m_shadowContrastSlider->value());
    m_shadowContrastSlider->blockSignals(false);
}

} // namespace zraw
