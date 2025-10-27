#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>

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
    
    // Setters to update UI (without triggering signals)
    void setExposure(float value);
    void setContrast(float value);
    void setSharpness(float value);

signals:
    void exposureChanged(float value);
    void contrastChanged(float value);
    void sharpnessChanged(float value);

private:
    QSlider* m_exposureSlider;
    QSlider* m_contrastSlider;
    QSlider* m_sharpnessSlider;
    
    QLabel* m_exposureLabel;
    QLabel* m_contrastLabel;
    QLabel* m_sharpnessLabel;
    
    void createUI();
    QWidget* createSliderGroup(const QString& name, QSlider** slider, QLabel** valueLabel,
                               int min, int max, int defaultVal);
    void updateExposureLabel(int value);
    void updateContrastLabel(int value);
    void updateSharpnessLabel(int value);
};

} // namespace zraw
