#pragma once

#include "ImageBuffer.h"
#include <string>
#include <memory>

// Forward declare LibRaw types
class LibRaw;

namespace zraw {

/**
 * Raw image processor using LibRaw
 * Handles loading and initial processing of RAW files
 */
class RawProcessor {
public:
    RawProcessor();
    ~RawProcessor();

    // Load raw file
    bool loadRaw(const std::string& filepath);
    
    // Process raw data to RGB
    bool processToRGB();
    
    // Get processed image buffer
    std::shared_ptr<ImageBuffer> getImageBuffer() const { return m_buffer; }
    
    // Get image metadata
    int width() const;
    int height() const;
    std::string cameraModel() const;
    float exposureTime() const;
    float aperture() const;
    int iso() const;
    
    // Get camera white balance (as-shot)
    float getCameraWBTemperature() const;  // Returns color temperature in Kelvin
    
    // Error handling
    std::string lastError() const { return m_lastError; }

private:
    std::unique_ptr<LibRaw> m_libraw;
    std::shared_ptr<ImageBuffer> m_buffer;
    std::string m_lastError;
    
    void setError(const std::string& error);
};

} // namespace zraw
