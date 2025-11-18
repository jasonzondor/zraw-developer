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
    
    // Configure output mode
    // When true, output is linear camera-space RGB (no gamma, output_color=0)
    // When false, output is LibRaw's sRGB (output_color=1, gamma-encoded)
    void setLinearOutput(bool enable);
    bool isLinearOutput() const { return m_linearOutput; }

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
    float getCameraWBTint() const;         // Returns tint (green-magenta shift)
    
    // Get raw camera WB multipliers (RGBG format)
    // These are the multipliers LibRaw would apply for camera WB
    // Returns: [R, G, B, G2] multipliers
    void getCameraWBMultipliers(float multipliers[4]) const;
    
    // Error handling
    std::string lastError() const { return m_lastError; }

private:
    std::unique_ptr<LibRaw> m_libraw;
    std::shared_ptr<ImageBuffer> m_buffer;
    std::string m_lastError;
    bool m_linearOutput{true};
    
    void setError(const std::string& error);
};

} // namespace zraw
