#include "AdjustmentPanel.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>

namespace zraw {

AdjustmentPanel::AdjustmentPanel(QWidget* parent)
    : QWidget(parent) {
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
    
    mainLayout->addStretch();
    
    // Initialize labels
    updateExposureLabel(0);
    updateContrastLabel(0);
    updateSharpnessLabel(0);
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
