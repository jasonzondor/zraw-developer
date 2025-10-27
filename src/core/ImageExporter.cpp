#include "ImageExporter.h"
#include <QImage>
#include <QFileInfo>
#include <iostream>

namespace zraw {

ImageExporter::ImageExporter() {
}

bool ImageExporter::exportImage(const std::shared_ptr<ImageBuffer>& buffer,
                                const QString& filepath,
                                Format format,
                                int quality) {
    if (!buffer || buffer->width() == 0 || buffer->height() == 0) {
        std::cerr << "Invalid image buffer" << std::endl;
        return false;
    }
    
    switch (format) {
        case Format::TIFF:
            return exportTIFF(buffer, filepath);
        case Format::JPEG:
            return exportJPEG(buffer, filepath, quality);
        case Format::PNG:
            return exportPNG(buffer, filepath);
    }
    
    return false;
}

ImageExporter::Format ImageExporter::formatFromExtension(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    QString ext = fileInfo.suffix().toLower();
    
    if (ext == "tiff" || ext == "tif") {
        return Format::TIFF;
    } else if (ext == "jpg" || ext == "jpeg") {
        return Format::JPEG;
    } else if (ext == "png") {
        return Format::PNG;
    }
    
    // Default to TIFF
    return Format::TIFF;
}

ImageExporter::Format ImageExporter::formatFromString(const QString& formatStr) {
    QString lower = formatStr.toLower();
    
    if (lower == "tiff" || lower == "tif") {
        return Format::TIFF;
    } else if (lower == "jpeg" || lower == "jpg") {
        return Format::JPEG;
    } else if (lower == "png") {
        return Format::PNG;
    }
    
    return Format::TIFF;
}

bool ImageExporter::exportTIFF(const std::shared_ptr<ImageBuffer>& buffer, const QString& filepath) {
    // Qt doesn't support 16-bit TIFF writing directly, so we'll use 8-bit for now
    // TODO: Use libtiff for proper 16-bit TIFF export
    auto data8bit = convertTo8Bit(buffer);
    
    QImage image(data8bit.data(), buffer->width(), buffer->height(), 
                 buffer->width() * 3, QImage::Format_RGB888);
    
    if (image.save(filepath, "TIFF")) {
        std::cout << "Exported TIFF: " << filepath.toStdString() << std::endl;
        return true;
    }
    
    std::cerr << "Failed to export TIFF" << std::endl;
    return false;
}

bool ImageExporter::exportJPEG(const std::shared_ptr<ImageBuffer>& buffer, 
                               const QString& filepath, int quality) {
    auto data8bit = convertTo8Bit(buffer);
    
    QImage image(data8bit.data(), buffer->width(), buffer->height(), 
                 buffer->width() * 3, QImage::Format_RGB888);
    
    if (image.save(filepath, "JPEG", quality)) {
        std::cout << "Exported JPEG: " << filepath.toStdString() 
                  << " (quality: " << quality << ")" << std::endl;
        return true;
    }
    
    std::cerr << "Failed to export JPEG" << std::endl;
    return false;
}

bool ImageExporter::exportPNG(const std::shared_ptr<ImageBuffer>& buffer, const QString& filepath) {
    auto data8bit = convertTo8Bit(buffer);
    
    QImage image(data8bit.data(), buffer->width(), buffer->height(), 
                 buffer->width() * 3, QImage::Format_RGB888);
    
    if (image.save(filepath, "PNG")) {
        std::cout << "Exported PNG: " << filepath.toStdString() << std::endl;
        return true;
    }
    
    std::cerr << "Failed to export PNG" << std::endl;
    return false;
}

std::vector<uint8_t> ImageExporter::convertTo8Bit(const std::shared_ptr<ImageBuffer>& buffer) {
    size_t pixelCount = buffer->width() * buffer->height() * 3;
    std::vector<uint8_t> data8bit(pixelCount);
    
    const uint16_t* src = buffer->data();
    uint8_t* dst = data8bit.data();
    
    // Convert 16-bit to 8-bit (simple right shift by 8)
    for (size_t i = 0; i < pixelCount; ++i) {
        dst[i] = src[i] >> 8;
    }
    
    return data8bit;
}

} // namespace zraw
