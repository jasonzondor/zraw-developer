#pragma once

#include "ImageBuffer.h"
#include <QString>
#include <memory>

namespace zraw {

/**
 * Export processed images to various formats
 */
class ImageExporter {
public:
    enum class Format {
        TIFF,
        JPEG,
        PNG
    };
    
    ImageExporter();
    
    /**
     * Export image buffer to file
     * @param buffer Image buffer (16-bit RGB)
     * @param filepath Output file path
     * @param format Output format
     * @param quality JPEG quality (1-100, ignored for TIFF/PNG)
     * @return true on success
     */
    bool exportImage(const std::shared_ptr<ImageBuffer>& buffer,
                    const QString& filepath,
                    Format format,
                    int quality = 95);
    
    /**
     * Get format from file extension
     */
    static Format formatFromExtension(const QString& filepath);
    
    /**
     * Get format from string
     */
    static Format formatFromString(const QString& formatStr);

private:
    bool exportTIFF(const std::shared_ptr<ImageBuffer>& buffer, const QString& filepath);
    bool exportJPEG(const std::shared_ptr<ImageBuffer>& buffer, const QString& filepath, int quality);
    bool exportPNG(const std::shared_ptr<ImageBuffer>& buffer, const QString& filepath);
    
    // Convert 16-bit to 8-bit for JPEG/PNG
    std::vector<uint8_t> convertTo8Bit(const std::shared_ptr<ImageBuffer>& buffer);
};

} // namespace zraw
