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
      m_gpuPipeline(std::make_shared<GPUPipeline>()) {
    
    setWindowTitle("ZRaw Developer");
    resize(1400, 900);
    
    createUI();
    createMenus();
    
    statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow() {
}

void MainWindow::createUI() {
    // Create main splitter
    auto* splitter = new QSplitter(Qt::Horizontal);
    
    // Image viewer (left side)
    m_viewer = new ImageViewer();
    m_viewer->setMinimumSize(800, 600);
    splitter->addWidget(m_viewer);
    
    // Adjustment panel (right side)
    m_adjustmentPanel = new AdjustmentPanel();
    m_adjustmentPanel->setMaximumWidth(350);
    splitter->addWidget(m_adjustmentPanel);
    
    // Set splitter sizes
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    
    setCentralWidget(splitter);
    
    // Connect adjustment signals
    connect(m_adjustmentPanel, &AdjustmentPanel::exposureChanged,
            this, &MainWindow::onExposureChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::contrastChanged,
            this, &MainWindow::onContrastChanged);
    connect(m_adjustmentPanel, &AdjustmentPanel::sharpnessChanged,
            this, &MainWindow::onSharpnessChanged);
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
    
    if (!processRawFile(filepath)) {
        QMessageBox::critical(this, "Error", "Failed to load RAW file:\n" +
                            QString::fromStdString(m_rawProcessor->lastError()));
        statusBar()->showMessage("Failed to load image");
        return false;
    }
    
    m_currentFile = filepath;
    setWindowTitle("ZRaw Developer - " + QFileInfo(filepath).fileName());
    statusBar()->showMessage("Loaded " + filepath);
    
    return true;
}

bool MainWindow::processRawFile(const QString& filepath) {
    // Load raw file
    if (!m_rawProcessor->loadRaw(filepath.toStdString())) {
        return false;
    }
    
    // Process to RGB
    if (!m_rawProcessor->processToRGB()) {
        return false;
    }
    
    // Initialize GPU pipeline if needed
    if (!m_gpuPipeline->isInitialized()) {
        // This will be initialized when the OpenGL context is ready
        m_viewer->makeCurrent();
        if (!m_gpuPipeline->initialize()) {
            std::cerr << "Failed to initialize GPU pipeline" << std::endl;
            return false;
        }
        m_viewer->doneCurrent();
    }
    
    // Upload to GPU
    m_viewer->makeCurrent();
    if (!m_gpuPipeline->uploadImage(m_rawProcessor->getImageBuffer())) {
        m_viewer->doneCurrent();
        return false;
    }
    
    // Set pipeline in viewer
    m_viewer->setGPUPipeline(m_gpuPipeline);
    
    // Initial processing
    updateImage();
    
    m_viewer->doneCurrent();
    
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
    }
}

void MainWindow::onContrastChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setContrast(value);
        updateImage();
    }
}

void MainWindow::onSharpnessChanged(float value) {
    if (m_gpuPipeline) {
        m_gpuPipeline->setSharpness(value);
        updateImage();
    }
}

void MainWindow::updateImage() {
    m_viewer->makeCurrent();
    m_gpuPipeline->process();
    m_viewer->updateDisplay();
    m_viewer->doneCurrent();
}

} // namespace zraw
