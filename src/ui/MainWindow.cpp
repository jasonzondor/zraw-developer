#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <iostream>

namespace zraw {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_rawProcessor(std::make_shared<RawProcessor>()),
      m_gpuPipeline(std::make_shared<GPUPipeline>()),
      m_xmpHandler(std::make_shared<XMPHandler>()),
      m_loadingXMP(false) {
    
    std::cout << "MainWindow constructor started" << std::endl;
    
    setWindowTitle("ZRaw Developer");
    resize(1400, 900);
    
    // Setup XMP save timer (debounce saves to 500ms after last change)
    m_xmpSaveTimer = new QTimer(this);
    m_xmpSaveTimer->setSingleShot(true);
    m_xmpSaveTimer->setInterval(500);  // 500ms delay
    connect(m_xmpSaveTimer, &QTimer::timeout, this, &MainWindow::saveXMPAdjustments);
    
    std::cout << "Creating UI..." << std::endl;
    createUI();
    
    std::cout << "Creating menus..." << std::endl;
    createMenus();
    
    statusBar()->showMessage("Ready");
    std::cout << "MainWindow constructor complete" << std::endl;
}

MainWindow::~MainWindow() {
}

void MainWindow::createUI() {
    // Create main splitter
    auto* splitter = new QSplitter(Qt::Horizontal);
    
    // Image viewer (left side)
    m_viewer = new ImageViewer();
    m_viewer->setMinimumSize(400, 300);  // Reduced minimum size
    splitter->addWidget(m_viewer);
    
    // Adjustment panel (right side)
    m_adjustmentPanel = new AdjustmentPanel();
    m_adjustmentPanel->setMinimumWidth(150);
    m_adjustmentPanel->setMaximumWidth(300);
    splitter->addWidget(m_adjustmentPanel);
    
    // Set splitter sizes
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    
    setCentralWidget(splitter);
    
    // Set minimum window size to prevent overlap
    setMinimumSize(650, 400);  // 400 (viewer min) + 150 (panel min) + margins
    
    // Connect adjustment signals
    connect(m_adjustmentPanel, &AdjustmentPanel::exposureChanged,
            this, &MainWindow::onExposureChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::contrastChanged,
            this, &MainWindow::onContrastChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::sharpnessChanged,
            this, &MainWindow::onSharpnessChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::temperatureChanged,
            this, &MainWindow::onTemperatureChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::tintChanged,
            this, &MainWindow::onTintChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::highlightsChanged,
            this, &MainWindow::onHighlightsChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::shadowsChanged,
            this, &MainWindow::onShadowsChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::vibranceChanged,
            this, &MainWindow::onVibranceChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::saturationChanged,
            this, &MainWindow::onSaturationChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::highlightContrastChanged,
            this, &MainWindow::onHighlightContrastChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::midtoneContrastChanged,
            this, &MainWindow::onMidtoneContrastChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::shadowContrastChanged,
            this, &MainWindow::onShadowContrastChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::whitesChanged,
            this, &MainWindow::onWhitesChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::blacksChanged,
            this, &MainWindow::onBlacksChanged);
}

void MainWindow::createMenus() {
    // File menu
    auto* fileMenu = menuBar()->addMenu("&File");
    
    auto* openAction = fileMenu->addAction("&Open RAW...");
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    
    auto* saveAction = fileMenu->addAction("&Save...");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    
    fileMenu->addSeparator();
    
    auto* quitAction = fileMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

void MainWindow::openFile() {
    QString filepath = QFileDialog::getOpenFileName(
        this,
        "Open RAW File",
        QString(),
        "RAW Files (*.cr2 *.nef *.arw *.dng *.raf *.orf *.rw2);;All Files (*)"
    );
    
    if (!filepath.isEmpty()) {
        loadImage(filepath);
    }
}

bool MainWindow::loadImage(const QString& filepath) {
    statusBar()->showMessage("Loading " + filepath + "...");
    
    // Set current file BEFORE processing so XMP loading works
    m_currentFile = filepath;
    
    if (!processRawFile(filepath)) {
        QMessageBox::critical(this, "Error", "Failed to load RAW file:\n" +
                            QString::fromStdString(m_rawProcessor->lastError()));
        statusBar()->showMessage("Failed to load image");
        m_currentFile.clear();  // Clear on failure
        return false;
    }
    
    setWindowTitle("ZRaw Developer - " + QFileInfo(filepath).fileName());
    statusBar()->showMessage("Loaded " + filepath);
    
    return true;
}

bool MainWindow::processRawFile(const QString& filepath) {
    std::cout << "Processing RAW file: " << filepath.toStdString() << std::endl;
    
    // Load raw file
    if (!m_rawProcessor->loadRaw(filepath.toStdString())) {
        std::cerr << "Failed to load RAW file" << std::endl;
        return false;
    }
    
    std::cout << "RAW file loaded, processing to RGB..." << std::endl;
    
    // Process to RGB
    if (!m_rawProcessor->processToRGB()) {
        std::cerr << "Failed to process to RGB" << std::endl;
        return false;
    }
    
    std::cout << "RGB processing complete" << std::endl;
    
    // Initialize GPU pipeline if needed
    if (!m_gpuPipeline->isInitialized()) {
        std::cout << "Initializing GPU pipeline..." << std::endl;
        // This will be initialized when the OpenGL context is ready
        m_viewer->makeCurrent();
        if (!m_gpuPipeline->initialize()) {
            std::cerr << "Failed to initialize GPU pipeline" << std::endl;
            return false;
        }
        m_viewer->doneCurrent();
        std::cout << "GPU pipeline initialized" << std::endl;
    }
    
    // Upload to GPU
    std::cout << "Uploading image to GPU..." << std::endl;
    m_viewer->makeCurrent();
    if (!m_gpuPipeline->uploadImage(m_rawProcessor->getImageBuffer())) {
        std::cerr << "Failed to upload image to GPU" << std::endl;
        m_viewer->doneCurrent();
        return false;
    }
    std::cout << "Image uploaded to GPU" << std::endl;
    
    // Set pipeline in viewer
    m_viewer->setGPUPipeline(m_gpuPipeline);
    
    m_viewer->doneCurrent();
    
    std::cout << "Loading XMP adjustments..." << std::endl;
    // Load XMP adjustments if they exist
    loadXMPAdjustments();
    
    std::cout << "Image processing complete" << std::endl;
    return true;
}

void MainWindow::saveFile() {
    if (m_currentFile.isEmpty()) {
        QMessageBox::information(this, "No Image", "Please load an image first.");
        return;
    }
    
    QString filepath = QFileDialog::getSaveFileName(
        this,
        "Save Image",
        QString(),
        "TIFF Files (*.tiff *.tif);;JPEG Files (*.jpg *.jpeg);;PNG Files (*.png)"
    );
    
    if (!filepath.isEmpty()) {
        statusBar()->showMessage("Saving to " + filepath + "...");
        
        // Download image from GPU
        m_viewer->makeCurrent();
        auto buffer = m_gpuPipeline->downloadImage();
        m_viewer->doneCurrent();
        
        if (buffer) {
            // TODO: Implement image saving
            statusBar()->showMessage("Image saved to " + filepath);
        } else {
            QMessageBox::critical(this, "Error", "Failed to save image");
            statusBar()->showMessage("Failed to save image");
        }
    }
}

void MainWindow::onExposureChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setExposure(value);
        updateImage();
        
        // Schedule debounced XMP save (unless we're loading)
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onContrastChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setContrast(value);
        updateImage();
        
        // Schedule debounced XMP save (unless we're loading)
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onSharpnessChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setSharpness(value);
        updateImage();
        
        // Schedule debounced XMP save (unless we're loading)
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onTemperatureChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setTemperature(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onTintChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setTint(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onHighlightsChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setHighlights(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onShadowsChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setShadows(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onVibranceChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setVibrance(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onSaturationChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setSaturation(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onHighlightContrastChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setHighlightContrast(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onMidtoneContrastChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setMidtoneContrast(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onShadowContrastChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setShadowContrast(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onWhitesChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setWhites(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::onBlacksChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setBlacks(value);
        updateImage();
        
        if (!m_loadingXMP) {
            scheduleXMPSave();
        }
    }
}

void MainWindow::updateImage() {
    m_viewer->makeCurrent();
    m_gpuPipeline->process();
    m_viewer->updateDisplay();
    m_viewer->doneCurrent();
}

void MainWindow::loadXMPAdjustments() {
    std::cout << "loadXMPAdjustments() called" << std::endl;
    
    if (m_currentFile.isEmpty() || !m_xmpHandler) {
        std::cout << "Skipping XMP load - no file or handler" << std::endl;
        return;
    }
    
    m_loadingXMP = true;  // Prevent auto-save while loading
    
    auto adjustments = m_xmpHandler->loadAdjustments(m_currentFile);
    
    std::cout << "XMP adjustments loaded: exp=" << adjustments.exposure 
              << " con=" << adjustments.contrast 
              << " sharp=" << adjustments.sharpness << std::endl;
    
    // Apply to GPU pipeline
    if (m_gpuPipeline) {
        m_gpuPipeline->setExposure(adjustments.exposure);
        m_gpuPipeline->setContrast(adjustments.contrast);
        m_gpuPipeline->setSharpness(adjustments.sharpness);
        m_gpuPipeline->setTemperature(adjustments.temperature);
        m_gpuPipeline->setTint(adjustments.tint);
        m_gpuPipeline->setHighlights(adjustments.highlights);
        m_gpuPipeline->setShadows(adjustments.shadows);
        m_gpuPipeline->setVibrance(adjustments.vibrance);
        m_gpuPipeline->setSaturation(adjustments.saturation);
        m_gpuPipeline->setHighlightContrast(adjustments.highlightContrast);
        m_gpuPipeline->setMidtoneContrast(adjustments.midtoneContrast);
        m_gpuPipeline->setShadowContrast(adjustments.shadowContrast);
    }
    
    // Update UI sliders
    if (m_adjustmentPanel) {
        m_adjustmentPanel->setExposure(adjustments.exposure);
        m_adjustmentPanel->setContrast(adjustments.contrast);
        m_adjustmentPanel->setSharpness(adjustments.sharpness);
        // Note: temperature and tint sliders removed (replaced with whites/blacks)
        m_adjustmentPanel->setHighlights(adjustments.highlights);
        m_adjustmentPanel->setShadows(adjustments.shadows);
        m_adjustmentPanel->setVibrance(adjustments.vibrance);
        m_adjustmentPanel->setSaturation(adjustments.saturation);
        m_adjustmentPanel->setHighlightContrast(adjustments.highlightContrast);
        m_adjustmentPanel->setMidtoneContrast(adjustments.midtoneContrast);
        m_adjustmentPanel->setShadowContrast(adjustments.shadowContrast);
        m_adjustmentPanel->setWhites(0);  // Default whites
        m_adjustmentPanel->setBlacks(0);  // Default blacks
    }
    
    m_loadingXMP = false;
    
    std::cout << "Calling updateImage()..." << std::endl;
    // Update display with loaded adjustments
    updateImage();
    
    std::cout << "updateImage() complete" << std::endl;
    
    if (m_xmpHandler->xmpExists(m_currentFile)) {
        statusBar()->showMessage("Loaded adjustments from XMP sidecar");
    }
}

void MainWindow::scheduleXMPSave() {
    // Restart the timer - this debounces rapid changes
    m_xmpSaveTimer->start();
}

void MainWindow::saveXMPAdjustments() {
    if (m_currentFile.isEmpty() || !m_xmpHandler || !m_adjustmentPanel) {
        return;
    }
    
    XMPHandler::Adjustments adjustments;
    adjustments.exposure = m_adjustmentPanel->exposure();
    adjustments.contrast = m_adjustmentPanel->contrast();
    adjustments.sharpness = m_adjustmentPanel->sharpness();
    adjustments.temperature = 0;  // Temperature slider removed
    adjustments.tint = 0;  // Tint slider removed
    adjustments.highlights = m_adjustmentPanel->highlights();
    adjustments.shadows = m_adjustmentPanel->shadows();
    adjustments.vibrance = m_adjustmentPanel->vibrance();
    adjustments.saturation = m_adjustmentPanel->saturation();
    adjustments.highlightContrast = m_adjustmentPanel->highlightContrast();
    adjustments.midtoneContrast = m_adjustmentPanel->midtoneContrast();
    adjustments.shadowContrast = m_adjustmentPanel->shadowContrast();
    
    m_xmpHandler->saveAdjustments(m_currentFile, adjustments);
}

} // namespace zraw
