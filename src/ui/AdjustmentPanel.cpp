#include "AdjustmentPanel.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <cmath>

namespace zraw {

AdjustmentPanel::AdjustmentPanel(QWidget* parent)
    : QWidget(parent),
      m_cameraWBKelvin(5500.0f),  // Default to daylight
      m_cameraWBTint(1.0f) {      // Default to neutral tint (Darktable scale: 1.0 = neutral)
    createUI();
}

AdjustmentPanel::~AdjustmentPanel() {
}

void AdjustmentPanel::createUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Set dark background
    setStyleSheet("AdjustmentPanel { background-color: #2b2b2b; }");
    
    // ========================================================================
    // SECTION 1: BASIC (Tone adjustments)
    // ========================================================================
    auto* basicContent = new QVBoxLayout();
    basicContent->setSpacing(8);
    basicContent->setContentsMargins(12, 8, 12, 8);
    
    // Exposure
    auto* exposureRow = createSliderRow("Exposure", &m_exposureSlider, &m_exposureLabel,
                                        -300, 300, 0);
    basicContent->addWidget(exposureRow);
    connect(m_exposureSlider, &QSlider::valueChanged, this, [this](int value) {
        updateExposureLabel(value);
        emit exposureChanged(value / 100.0f);
    });
    
    // Contrast
    auto* contrastRow = createSliderRow("Contrast", &m_contrastSlider, &m_contrastLabel,
                                        -100, 100, 0);
    basicContent->addWidget(contrastRow);
    connect(m_contrastSlider, &QSlider::valueChanged, this, [this](int value) {
        updateContrastLabel(value);
        emit contrastChanged(value / 100.0f);
    });
    
    // Highlights
    auto* highlightsRow = createSliderRow("Highlights", &m_highlightsSlider, &m_highlightsLabel,
                                          -100, 100, 0);
    basicContent->addWidget(highlightsRow);
    connect(m_highlightsSlider, &QSlider::valueChanged, this, [this](int value) {
        updateHighlightsLabel(value);
        emit highlightsChanged(static_cast<float>(value));
    });
    
    // Shadows
    auto* shadowsRow = createSliderRow("Shadows", &m_shadowsSlider, &m_shadowsLabel,
                                       -100, 100, 0);
    basicContent->addWidget(shadowsRow);
    connect(m_shadowsSlider, &QSlider::valueChanged, this, [this](int value) {
        updateShadowsLabel(value);
        emit shadowsChanged(static_cast<float>(value));
    });
    
    // Whites
    auto* whiteRow = createSliderRow("Whites", &m_whitesSlider, &m_whitesLabel,
                                     -100, 100, 0);
    basicContent->addWidget(whiteRow);
    connect(m_whitesSlider, &QSlider::valueChanged, this, [this](int value) {
        updateWhitesLabel(value);
        emit whitesChanged(static_cast<float>(value));
    });
    
    // Blacks
    auto* blackRow = createSliderRow("Blacks", &m_blacksSlider, &m_blacksLabel,
                                     -100, 100, 0);
    basicContent->addWidget(blackRow);
    connect(m_blacksSlider, &QSlider::valueChanged, this, [this](int value) {
        updateBlacksLabel(value);
        emit blacksChanged(static_cast<float>(value));
    });
    
    auto* basicSection = createSection("Basic", basicContent);
    mainLayout->addWidget(basicSection);
    
    // ========================================================================
    // SECTION 2: COLOR
    // ========================================================================
    auto* colorContent = new QVBoxLayout();
    colorContent->setSpacing(8);
    colorContent->setContentsMargins(12, 8, 12, 8);
    
    // Temperature (White Balance) - Kelvin values (2000K to 10000K)
    auto* temperatureRow = createSliderRow("Temperature", &m_temperatureSlider, &m_temperatureLabel,
                                           2000, 10000, 5500);  // 2000K to 10000K, default 5500K (daylight)
    colorContent->addWidget(temperatureRow);
    connect(m_temperatureSlider, &QSlider::valueChanged, this, [this](int value) {
        updateTemperatureLabel(value);
        // Emit relative adjustment from camera WB
        // Convert Kelvin difference to shader range (-100 to +100)
        // Map ±2000K to ±100 (20K per unit)
        float kelvinDiff = static_cast<float>(value) - m_cameraWBKelvin;
        float relativeAdjustment = kelvinDiff / 20.0f;
        // Clamp to reasonable range
        if (relativeAdjustment < -100.0f) relativeAdjustment = -100.0f;
        if (relativeAdjustment > 100.0f) relativeAdjustment = 100.0f;
        emit temperatureChanged(relativeAdjustment);
    });
    
    // Tint (White Balance) - Darktable scale: 0.5 to 1.5, with 1.0 = neutral
    // Use integer slider 500-1500 (divide by 1000 for display)
    auto* tintRow = createSliderRow("Tint", &m_tintSlider, &m_tintLabel,
                                    500, 1500, 1000);  // 0.5 to 1.5, default 1.0
    colorContent->addWidget(tintRow);
    connect(m_tintSlider, &QSlider::valueChanged, this, [this](int value) {
        updateTintLabel(value);
        // Convert to Darktable scale (0.5 to 1.5)
        float tintValue = value / 1000.0f;
        // Convert to shader scale: (tint - 1.0) * 100 maps to -50 to +50
        float shaderTint = (tintValue - 1.0f) * 100.0f;
        emit tintChanged(shaderTint);
    });
    
    // Vibrance
    auto* vibranceRow = createSliderRow("Vibrance", &m_vibranceSlider, &m_vibranceLabel,
                                        -100, 100, 0);
    colorContent->addWidget(vibranceRow);
    connect(m_vibranceSlider, &QSlider::valueChanged, this, [this](int value) {
        updateVibranceLabel(value);
        emit vibranceChanged(static_cast<float>(value));
    });
    
    // Saturation
    auto* saturationRow = createSliderRow("Saturation", &m_saturationSlider, &m_saturationLabel,
                                          -100, 100, 0);
    colorContent->addWidget(saturationRow);
    connect(m_saturationSlider, &QSlider::valueChanged, this, [this](int value) {
        updateSaturationLabel(value);
        emit saturationChanged(static_cast<float>(value));
    });
    
    auto* colorSection = createSection("Color", colorContent);
    mainLayout->addWidget(colorSection);
    
    // ========================================================================
    // SECTION 3: DETAIL
    // ========================================================================
    auto* detailContent = new QVBoxLayout();
    detailContent->setSpacing(8);
    detailContent->setContentsMargins(12, 8, 12, 8);
    
    // Sharpness
    auto* sharpnessRow = createSliderRow("Sharpness", &m_sharpnessSlider, &m_sharpnessLabel,
                                         0, 200, 0);
    detailContent->addWidget(sharpnessRow);
    connect(m_sharpnessSlider, &QSlider::valueChanged, this, [this](int value) {
        updateSharpnessLabel(value);
        emit sharpnessChanged(value / 100.0f);
    });
    
    auto* detailSection = createSection("Detail", detailContent);
    mainLayout->addWidget(detailSection);
    
    // ========================================================================
    // SECTION 4: ADVANCED (Local contrast controls)
    // ========================================================================
    auto* advancedContent = new QVBoxLayout();
    advancedContent->setSpacing(8);
    advancedContent->setContentsMargins(12, 8, 12, 8);
    
    // Highlight Contrast
    auto* highlightContrastRow = createSliderRow("Highlight Contrast", &m_highlightContrastSlider,
                                                 &m_highlightContrastLabel, -100, 100, 0);
    advancedContent->addWidget(highlightContrastRow);
    connect(m_highlightContrastSlider, &QSlider::valueChanged, this, [this](int value) {
        updateHighlightContrastLabel(value);
        emit highlightContrastChanged(static_cast<float>(value));
    });
    
    // Midtone Contrast
    auto* midtoneContrastRow = createSliderRow("Midtone Contrast", &m_midtoneContrastSlider,
                                               &m_midtoneContrastLabel, -100, 100, 0);
    advancedContent->addWidget(midtoneContrastRow);
    connect(m_midtoneContrastSlider, &QSlider::valueChanged, this, [this](int value) {
        updateMidtoneContrastLabel(value);
        emit midtoneContrastChanged(static_cast<float>(value));
    });
    
    // Shadow Contrast
    auto* shadowContrastRow = createSliderRow("Shadow Contrast", &m_shadowContrastSlider,
                                              &m_shadowContrastLabel, -100, 100, 0);
    advancedContent->addWidget(shadowContrastRow);
    connect(m_shadowContrastSlider, &QSlider::valueChanged, this, [this](int value) {
        updateShadowContrastLabel(value);
        emit shadowContrastChanged(static_cast<float>(value));
    });
    
    auto* advancedSection = createSection("Advanced", advancedContent);
    mainLayout->addWidget(advancedSection);
    
    // ========================================================================
    // CROP SECTION
    // ========================================================================
    auto* cropContent = new QVBoxLayout();
    cropContent->setSpacing(8);
    cropContent->setContentsMargins(12, 8, 12, 8);
    
    // Crop enable button
    m_cropButton = new QPushButton("Enable Crop");
    m_cropButton->setCheckable(true);
    m_cropButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #2a2a2a;"
        "  color: #e0e0e0;"
        "  border: 1px solid #3a3a3a;"
        "  border-radius: 4px;"
        "  padding: 6px 12px;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #3a3a3a;"
        "}"
        "QPushButton:checked {"
        "  background-color: #4a9eff;"
        "  border-color: #4a9eff;"
        "}"
    );
    connect(m_cropButton, &QPushButton::toggled, this, &AdjustmentPanel::cropModeChanged);
    cropContent->addWidget(m_cropButton);
    
    // Aspect ratio selector
    auto* aspectRow = new QWidget();
    auto* aspectLayout = new QHBoxLayout(aspectRow);
    aspectLayout->setContentsMargins(0, 0, 0, 0);
    
    auto* aspectLabel = new QLabel("Aspect:");
    aspectLabel->setStyleSheet("QLabel { color: #b0b0b0; font-size: 12px; }");
    aspectLayout->addWidget(aspectLabel);
    
    m_aspectRatioCombo = new QComboBox();
    m_aspectRatioCombo->addItem("Free", 0.0f);
    m_aspectRatioCombo->addItem("Original", -1.0f);  // Special value for image aspect
    m_aspectRatioCombo->addItem("16:9", 16.0f/9.0f);
    m_aspectRatioCombo->addItem("4:3", 4.0f/3.0f);
    m_aspectRatioCombo->addItem("3:2", 3.0f/2.0f);
    m_aspectRatioCombo->addItem("1:1", 1.0f);
    m_aspectRatioCombo->setCurrentIndex(1);  // Default to "Original"
    connect(m_aspectRatioCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, [this](int index) {
        float ratio = m_aspectRatioCombo->itemData(index).toFloat();
        emit aspectRatioChanged(ratio);
    });
    m_aspectRatioCombo->setStyleSheet(
        "QComboBox {"
        "  background-color: #2a2a2a;"
        "  color: #e0e0e0;"
        "  border: 1px solid #3a3a3a;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  font-size: 12px;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "}"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 4px solid transparent;"
        "  border-right: 4px solid transparent;"
        "  border-top: 6px solid #e0e0e0;"
        "  margin-right: 6px;"
        "}"
    );
    aspectLayout->addWidget(m_aspectRatioCombo);
    cropContent->addWidget(aspectRow);
    
    // Orientation selector
    auto* orientRow = new QWidget();
    auto* orientLayout = new QHBoxLayout(orientRow);
    orientLayout->setContentsMargins(0, 0, 0, 0);
    
    auto* orientLabel = new QLabel("Orient:");
    orientLabel->setStyleSheet("QLabel { color: #b0b0b0; font-size: 12px; }");
    orientLayout->addWidget(orientLabel);
    
    auto* orientCombo = new QComboBox();
    orientCombo->addItem("Landscape", false);  // false = not swapped
    orientCombo->addItem("Portrait", true);    // true = swapped
    orientCombo->setStyleSheet(
        "QComboBox {"
        "  background-color: #2a2a2a;"
        "  color: #e0e0e0;"
        "  border: 1px solid #3a3a3a;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  font-size: 12px;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "}"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 4px solid transparent;"
        "  border-right: 4px solid transparent;"
        "  border-top: 6px solid #e0e0e0;"
        "  margin-right: 6px;"
        "}"
    );
    connect(orientCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, orientCombo](int index) {
        bool isPortrait = orientCombo->itemData(index).toBool();
        emit swapOrientationChanged(isPortrait);
    });
    orientLayout->addWidget(orientCombo);
    cropContent->addWidget(orientRow);
    
    // Reset crop button
    m_cropResetButton = new QPushButton("Reset Crop");
    m_cropResetButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #2a2a2a;"
        "  color: #e0e0e0;"
        "  border: 1px solid #3a3a3a;"
        "  border-radius: 4px;"
        "  padding: 6px 12px;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #3a3a3a;"
        "}"
    );
    connect(m_cropResetButton, &QPushButton::clicked, this, &AdjustmentPanel::cropReset);
    cropContent->addWidget(m_cropResetButton);
    
    auto* cropSection = createSection("Crop", cropContent);
    mainLayout->addWidget(cropSection);
    
    mainLayout->addStretch();
    
    // Initialize labels
    updateExposureLabel(0);
    updateContrastLabel(0);
    updateSharpnessLabel(0);
    updateTemperatureLabel(5500);  // Default daylight
    updateTintLabel(1000);  // Default neutral (1.0)
    updateHighlightsLabel(0);
    updateShadowsLabel(0);
    updateVibranceLabel(0);
    updateSaturationLabel(0);
    updateHighlightContrastLabel(0);
    updateMidtoneContrastLabel(0);
    updateShadowContrastLabel(0);
    updateWhitesLabel(0);
    updateBlacksLabel(0);
}

QWidget* AdjustmentPanel::createSection(const QString& title, QVBoxLayout* contentLayout) {
    auto* section = new QWidget();
    auto* sectionLayout = new QVBoxLayout(section);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(0);
    
    // Header with collapse button
    auto* header = new QWidget();
    header->setStyleSheet("QWidget { background-color: #3a3a3a; }");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 8, 12, 8);
    
    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("QLabel { color: #e0e0e0; font-weight: bold; font-size: 13px; }");
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    auto* collapseBtn = new QPushButton("▼");
    collapseBtn->setFixedSize(20, 20);
    collapseBtn->setStyleSheet(
        "QPushButton { "
        "  background-color: transparent; "
        "  border: none; "
        "  color: #888; "
        "  font-size: 10px; "
        "}"
        "QPushButton:hover { color: #fff; }"
    );
    headerLayout->addWidget(collapseBtn);
    
    sectionLayout->addWidget(header);
    
    // Content widget
    auto* contentWidget = new QWidget();
    contentWidget->setLayout(contentLayout);
    contentWidget->setStyleSheet("QWidget { background-color: #2b2b2b; }");
    sectionLayout->addWidget(contentWidget);
    
    // Connect collapse button
    connect(collapseBtn, &QPushButton::clicked, [collapseBtn, contentWidget]() {
        bool isVisible = contentWidget->isVisible();
        contentWidget->setVisible(!isVisible);
        collapseBtn->setText(isVisible ? "▶" : "▼");
    });
    
    // Add separator line
    auto* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("QFrame { background-color: #1a1a1a; max-height: 1px; }");
    sectionLayout->addWidget(separator);
    
    return section;
}

QWidget* AdjustmentPanel::createSliderRow(const QString& name, QSlider** slider,
                                          QLabel** valueLabel, int min, int max, int defaultVal) {
    auto* row = new QWidget();
    auto* rowLayout = new QVBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(4);
    
    // Top row: label and value
    auto* topRow = new QWidget();
    auto* topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);
    
    auto* nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("QLabel { color: #b0b0b0; font-size: 12px; }");
    topLayout->addWidget(nameLabel);
    
    topLayout->addStretch();
    
    *valueLabel = new QLabel("0");
    (*valueLabel)->setStyleSheet("QLabel { color: #e0e0e0; font-size: 12px; }");
    (*valueLabel)->setAlignment(Qt::AlignRight);
    topLayout->addWidget(*valueLabel);
    
    rowLayout->addWidget(topRow);
    
    // Slider (with double-click reset support)
    auto* resettableSlider = new ResettableSlider(Qt::Horizontal);
    resettableSlider->setMinimum(min);
    resettableSlider->setMaximum(max);
    resettableSlider->setValue(defaultVal);
    resettableSlider->setDefaultValue(defaultVal);
    *slider = resettableSlider;
    (*slider)->setStyleSheet(
        "QSlider::groove:horizontal {"
        "  background: #1a1a1a;"
        "  height: 4px;"
        "  border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: #ffffff;"
        "  width: 12px;"
        "  height: 12px;"
        "  margin: -4px 0;"
        "  border-radius: 6px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "  background: #e0e0e0;"
        "}"
        "QSlider::sub-page:horizontal {"
        "  background: #4a9eff;"
        "  border-radius: 2px;"
        "}"
    );
    rowLayout->addWidget(*slider);
    
    return row;
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
    // Show Kelvin value with indicator if it matches camera WB
    QString text = QString("%1K").arg(value);
    
    // Add indicator if this is close to camera WB (within 50K)
    if (std::abs(static_cast<float>(value) - m_cameraWBKelvin) < 50.0f) {
        text += "";
    }
    
    if (m_temperatureLabel) {
        m_temperatureLabel->setText(text);
    }
}

void AdjustmentPanel::updateTintLabel(int value) {
    // Convert from slider value (500-1500) to display value (0.5-1.5)
    float tintValue = value / 1000.0f;
    QString text = QString::number(tintValue, 'f', 3);  // 3 decimal places
    
    // Add indicator if this is close to camera tint (within 0.01)
    if (std::abs(tintValue - m_cameraWBTint) < 0.01f) {
        text += "";
    }
    
    if (m_tintLabel) {
        m_tintLabel->setText(text);
    }
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

void AdjustmentPanel::updateWhitesLabel(int value) {
    m_whitesLabel->setText(QString::number(value));
}

void AdjustmentPanel::updateBlacksLabel(int value) {
    m_blacksLabel->setText(QString::number(value));
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
    // Return relative adjustment from camera WB, scaled to shader range
    float kelvinDiff = static_cast<float>(m_temperatureSlider->value()) - m_cameraWBKelvin;
    float relativeAdjustment = kelvinDiff / 20.0f;
    // Clamp to reasonable range
    if (relativeAdjustment < -100.0f) relativeAdjustment = -100.0f;
    if (relativeAdjustment > 100.0f) relativeAdjustment = 100.0f;
    return relativeAdjustment;
}

float AdjustmentPanel::tint() const {
    // Convert from slider value (500-1500) to Darktable scale (0.5-1.5)
    float tintValue = m_tintSlider->value() / 1000.0f;
    // Convert to shader scale: (tint - 1.0) * 100
    return (tintValue - 1.0f) * 100.0f;
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

float AdjustmentPanel::whites() const {
    return static_cast<float>(m_whitesSlider->value());
}

float AdjustmentPanel::blacks() const {
    return static_cast<float>(m_blacksSlider->value());
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
    // Value is relative adjustment in shader range (-100 to +100)
    // Convert back to Kelvin: value * 20K per unit
    float kelvinDiff = value * 20.0f;
    int kelvin = static_cast<int>(m_cameraWBKelvin + kelvinDiff);
    m_temperatureSlider->blockSignals(true);
    m_temperatureSlider->setValue(kelvin);
    updateTemperatureLabel(m_temperatureSlider->value());
    m_temperatureSlider->blockSignals(false);
}

void AdjustmentPanel::setCameraWBKelvin(float kelvin) {
    m_cameraWBKelvin = kelvin;
    
    // Update the default value of the slider to the camera WB
    if (auto* resettableSlider = dynamic_cast<ResettableSlider*>(m_temperatureSlider)) {
        resettableSlider->setDefaultValue(static_cast<int>(kelvin));
    }
}

void AdjustmentPanel::setCameraWBTint(float tint) {
    m_cameraWBTint = tint;  // Store in Darktable scale (0.5-1.5)
    
    // Convert to slider value (500-1500)
    int sliderValue = static_cast<int>(tint * 1000.0f);
    
    // Update the default value of the slider to the camera tint
    if (auto* resettableSlider = dynamic_cast<ResettableSlider*>(m_tintSlider)) {
        resettableSlider->setDefaultValue(sliderValue);
    }
}

void AdjustmentPanel::setTint(float value) {
    // Value is in shader scale (-50 to +50)
    // Convert back to Darktable scale: value/100 + 1.0
    float tintValue = (value / 100.0f) + 1.0f;
    // Convert to slider value (500-1500)
    int sliderValue = static_cast<int>(tintValue * 1000.0f);
    
    m_tintSlider->blockSignals(true);
    m_tintSlider->setValue(sliderValue);
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

void AdjustmentPanel::setWhites(float value) {
    m_whitesSlider->blockSignals(true);
    m_whitesSlider->setValue(static_cast<int>(value));
    updateWhitesLabel(m_whitesSlider->value());
    m_whitesSlider->blockSignals(false);
}

void AdjustmentPanel::setBlacks(float value) {
    m_blacksSlider->blockSignals(true);
    m_blacksSlider->setValue(static_cast<int>(value));
    updateBlacksLabel(m_blacksSlider->value());
    m_blacksSlider->blockSignals(false);
}

} // namespace zraw
