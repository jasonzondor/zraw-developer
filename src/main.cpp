#include <QApplication>
#include <QSurfaceFormat>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <iostream>
#include "ui/MainWindow.h"
#include "core/CLIHandler.h"
#include "core/RawProcessor.h"
#include "core/ImageExporter.h"
#include "gpu/GPUPipeline.h"

// Headless processing mode
int runHeadless(const zraw::CLIHandler::Options& options) {
    // Create offscreen OpenGL context for GPU processing
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    
    QOpenGLContext context;
    context.setFormat(format);
    if (!context.create()) {
        std::cerr << "Failed to create OpenGL context" << std::endl;
        return 1;
    }
    
    QOffscreenSurface surface;
    surface.setFormat(format);
    surface.create();
    
    if (!surface.isValid()) {
        std::cerr << "Failed to create offscreen surface" << std::endl;
        return 1;
    }
    
    if (!context.makeCurrent(&surface)) {
        std::cerr << "Failed to make OpenGL context current" << std::endl;
        return 1;
    }
    
    std::cout << "OpenGL context created successfully" << std::endl;
    
    std::cout << "Processing: " << options.inputFile.toStdString() << std::endl;
    
    // Load RAW file
    auto rawProcessor = std::make_shared<zraw::RawProcessor>();
    if (!rawProcessor->loadRaw(options.inputFile.toStdString())) {
        std::cerr << "Failed to load RAW file: " << rawProcessor->lastError() << std::endl;
        return 1;
    }
    
    if (!rawProcessor->processToRGB()) {
        std::cerr << "Failed to process RAW data: " << rawProcessor->lastError() << std::endl;
        return 1;
    }
    
    // Create GPU pipeline
    auto gpuPipeline = std::make_shared<zraw::GPUPipeline>();
    if (!gpuPipeline->initialize()) {
        std::cerr << "Failed to initialize GPU pipeline" << std::endl;
        return 1;
    }
    
    if (!gpuPipeline->uploadImage(rawProcessor->getImageBuffer())) {
        std::cerr << "Failed to upload image to GPU" << std::endl;
        return 1;
    }
    
    // Apply adjustments
    gpuPipeline->setExposure(options.exposure);
    gpuPipeline->setContrast(options.contrast);
    gpuPipeline->setSharpness(options.sharpness);
    
    std::cout << "Applying adjustments:" << std::endl;
    std::cout << "  Exposure:  " << options.exposure << " EV" << std::endl;
    std::cout << "  Contrast:  " << options.contrast << std::endl;
    std::cout << "  Sharpness: " << options.sharpness << std::endl;
    
    // Process
    if (!gpuPipeline->process()) {
        std::cerr << "Failed to process image" << std::endl;
        return 1;
    }
    
    // Download processed image
    auto processedBuffer = gpuPipeline->downloadImage();
    if (!processedBuffer) {
        std::cerr << "Failed to download processed image" << std::endl;
        return 1;
    }
    
    // Export
    zraw::ImageExporter exporter;
    auto format_enum = zraw::ImageExporter::formatFromString(options.format);
    
    if (!exporter.exportImage(processedBuffer, options.outputFile, format_enum, options.quality)) {
        std::cerr << "Failed to export image" << std::endl;
        return 1;
    }
    
    std::cout << "Successfully processed and exported image" << std::endl;
    return 0;
}

// GUI mode (app already created in main)
int runGUI(const zraw::CLIHandler::Options& options) {
    // Set OpenGL format
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);
    
    // Create main window
    zraw::MainWindow window;
    window.show();
    
    // Load image if provided
    if (!options.inputFile.isEmpty()) {
        if (!window.loadImage(options.inputFile)) {
            std::cerr << "Failed to load image: " << options.inputFile.toStdString() << std::endl;
        }
    }
    
    return qApp->exec();
}

int main(int argc, char* argv[]) {
    std::cout << "Starting ZRaw Developer..." << std::endl;
    
    QCoreApplication::setApplicationName("ZRaw Developer");
    QCoreApplication::setApplicationVersion("0.1.0");
    
    // Quick check for headless mode before creating Qt app
    bool isHeadless = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--headless") {
            isHeadless = true;
            break;
        }
    }
    
    if (isHeadless) {
        std::cout << "Running in headless mode" << std::endl;
        // Headless mode - use QCoreApplication
        QCoreApplication app(argc, argv);
        
        zraw::CLIHandler cli;
        if (!cli.parse(QCoreApplication::arguments())) {
            std::cerr << cli.helpText().toStdString() << std::endl;
            return 1;
        }
        
        return runHeadless(cli.options());
    } else {
        std::cout << "Running in GUI mode" << std::endl;
        // GUI mode or help - use QApplication
        QApplication app(argc, argv);
        
        std::cout << "Parsing command line..." << std::endl;
        zraw::CLIHandler cli;
        if (!cli.parse(QCoreApplication::arguments())) {
            std::cerr << cli.helpText().toStdString() << std::endl;
            return 1;
        }
        
        std::cout << "Command line parsed" << std::endl;
        
        if (cli.helpRequested()) {
            std::cout << cli.helpText().toStdString() << std::endl;
            return 0;
        }
        
        std::cout << "Starting GUI..." << std::endl;
        return runGUI(cli.options());
    }
}
