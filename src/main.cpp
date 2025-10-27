#include <QApplication>
#include <QSurfaceFormat>
#include <iostream>
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
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
    
    // Load image from command line if provided
    if (argc > 1) {
        QString filepath = QString::fromLocal8Bit(argv[1]);
        if (!window.loadImage(filepath)) {
            std::cerr << "Failed to load image: " << filepath.toStdString() << std::endl;
        }
    }
    
    return app.exec();
}
