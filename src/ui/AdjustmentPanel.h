#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>

namespace zraw {

/**
 * Panel for adjustment controls
 */
class AdjustmentPanel : public QWidget {
    Q_OBJECT

public:
    explicit AdjustmentPanel(QWidget* parent = nullptr);
    ~AdjustmentPanel();
    
    // Getters for current values
    float exposure() const;
    float contrast() const;
    float sharpness() const;
    float temperature() const;
    float tint() const;
    float highlights() const;
    float shadows() const;
    float vibrance() const;
    float saturation() const;
    float highlightContrast() const;
    float midtoneContrast() const;
    float shadowContrast() const;
    float whites() const;
    float blacks() const;
    
    // Setters to update UI (without triggering signals)
    void setExposure(float value);
    void setContrast(float value);
    void setSharpness(float value);
    void setTemperature(float value);
    void setTint(float value);
    void setHighlights(float value);
    void setShadows(float value);
    void setVibrance(float value);
    void setSaturation(float value);
    void setHighlightContrast(float value);
    void setMidtoneContrast(float value);
    void setShadowContrast(float value);
    void setWhites(float value);
    void setBlacks(float value);

signals:
    void exposureChanged(float value);
    void contrastChanged(float value);
    void sharpnessChanged(float value);
    void temperatureChanged(float value);
    void tintChanged(float value);
    void highlightsChanged(float value);
    void shadowsChanged(float value);
    void vibranceChanged(float value);
    void saturationChanged(float value);
    void highlightContrastChanged(float value);
    void midtoneContrastChanged(float value);
    void shadowContrastChanged(float value);
    void whitesChanged(float value);
    void blacksChanged(float value);

private:
    QSlider* m_exposureSlider;
    QSlider* m_contrastSlider;
    QSlider* m_sharpnessSlider;
    QSlider* m_temperatureSlider;
    QSlider* m_tintSlider;
    QSlider* m_highlightsSlider;
    QSlider* m_shadowsSlider;
    QSlider* m_vibranceSlider;
    QSlider* m_saturationSlider;
    QSlider* m_highlightContrastSlider;
    QSlider* m_midtoneContrastSlider;
    QSlider* m_shadowContrastSlider;
    QSlider* m_whitesSlider;
    QSlider* m_blacksSlider;
    
    QLabel* m_exposureLabel;
    QLabel* m_contrastLabel;
    QLabel* m_sharpnessLabel;
    QLabel* m_temperatureLabel;
    QLabel* m_tintLabel;
    QLabel* m_highlightsLabel;
    QLabel* m_shadowsLabel;
    QLabel* m_vibranceLabel;
    QLabel* m_saturationLabel;
    QLabel* m_highlightContrastLabel;
    QLabel* m_midtoneContrastLabel;
    QLabel* m_shadowContrastLabel;
    QLabel* m_whitesLabel;
    QLabel* m_blacksLabel;
    
    void createUI();
    QWidget* createSection(const QString& title, QVBoxLayout* contentLayout);
    QWidget* createSliderRow(const QString& name, QSlider** slider, QLabel** valueLabel,
                             int min, int max, int defaultVal);
    void updateExposureLabel(int value);
    void updateContrastLabel(int value);
    void updateSharpnessLabel(int value);
    void updateTemperatureLabel(int value);
    void updateTintLabel(int value);
    void updateHighlightsLabel(int value);
    void updateShadowsLabel(int value);
    void updateVibranceLabel(int value);
    void updateSaturationLabel(int value);
    void updateHighlightContrastLabel(int value);
    void updateMidtoneContrastLabel(int value);
    void updateShadowContrastLabel(int value);
    void updateWhitesLabel(int value);
    void updateBlacksLabel(int value);
};

} // namespace zraw
