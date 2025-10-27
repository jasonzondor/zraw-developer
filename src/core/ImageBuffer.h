#pragma once

#include <vector>
#include <cstdint>
#include <memory>

namespace zraw {

/**
 * Image buffer for storing raw and processed image data
 * Supports 16-bit per channel RGB data for high dynamic range
 */
class ImageBuffer {
public:
    ImageBuffer();
    ImageBuffer(int width, int height, int channels = 3);
    ~ImageBuffer();

    // Getters
    int width() const { return m_width; }
    int height() const { return m_height; }
    int channels() const { return m_channels; }
    size_t size() const { return m_data.size(); }
    
    // Data access
    uint16_t* data() { return m_data.data(); }
    const uint16_t* data() const { return m_data.data(); }
    
    // Allocate buffer
    void allocate(int width, int height, int channels = 3);
    
    // Copy data
    void copyFrom(const uint16_t* src, size_t count);
    
    // Convert to 8-bit for display
    std::vector<uint8_t> to8bit() const;
    
    // Clear buffer
    void clear();

private:
    int m_width;
    int m_height;
    int m_channels;
    std::vector<uint16_t> m_data;
};

} // namespace zraw
