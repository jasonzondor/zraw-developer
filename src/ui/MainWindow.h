#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <memory>
#include "ImageViewer.h"
#include "AdjustmentPanel.h"
#include "../core/RawProcessor.h"
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

private:
    ImageViewer* m_viewer;
    AdjustmentPanel* m_adjustmentPanel;
    
    std::shared_ptr<RawProcessor> m_rawProcessor;
    std::shared_ptr<GPUPipeline> m_gpuPipeline;
    
    QString m_currentFile;
    
    void createUI();
    void createMenus();
    void updateImage();
    bool processRawFile(const QString& filepath);
};

} // namespace zraw
