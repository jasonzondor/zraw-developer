#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <memory>
#include "ImageViewer.h"
#include "AdjustmentPanel.h"
#include "../core/RawProcessor.h"
#include "../core/XMPHandler.h"
#include "../gpu/GPUPipeline.h"

namespace zraw {

/**
 * Main application window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Load image from command line
    bool loadImage(const QString& filepath);

private slots:
    void openFile();
    void saveFile();
    void onExposureChanged(float value);
    void onContrastChanged(float value);
    void onSharpnessChanged(float value);
    void onTemperatureChanged(float value);
    void onTintChanged(float value);
    void onHighlightsChanged(float value);
    void onShadowsChanged(float value);
    void onVibranceChanged(float value);
    void onSaturationChanged(float value);
    void onHighlightContrastChanged(float value);
    void onMidtoneContrastChanged(float value);
    void onShadowContrastChanged(float value);

private:
    ImageViewer* m_viewer;
    AdjustmentPanel* m_adjustmentPanel;
    
    std::shared_ptr<RawProcessor> m_rawProcessor;
    std::shared_ptr<GPUPipeline> m_gpuPipeline;
    std::shared_ptr<XMPHandler> m_xmpHandler;
    
    QString m_currentFile;
    bool m_loadingXMP;  // Flag to prevent saving while loading
    QTimer* m_xmpSaveTimer;  // Timer for debounced XMP saving
    
    void createUI();
    void createMenus();
    void updateImage();
    bool processRawFile(const QString& filepath);
    void loadXMPAdjustments();
    void saveXMPAdjustments();
    void scheduleXMPSave();  // Schedule a debounced save
};

} // namespace zraw
