#include "ImageBuffer.h"
#include <algorithm>
#include <cstring>

namespace zraw {

ImageBuffer::ImageBuffer()
    : m_width(0), m_height(0), m_channels(3) {
}

ImageBuffer::ImageBuffer(int width, int height, int channels)
    : m_width(width), m_height(height), m_channels(channels) {
    allocate(width, height, channels);
}

ImageBuffer::~ImageBuffer() {
}

void ImageBuffer::allocate(int width, int height, int channels) {
    m_width = width;
    m_height = height;
    m_channels = channels;
    m_data.resize(static_cast<size_t>(width) * height * channels);
}

void ImageBuffer::copyFrom(const uint16_t* src, size_t count) {
    if (count > m_data.size()) {
        count = m_data.size();
    }
    std::memcpy(m_data.data(), src, count * sizeof(uint16_t));
}

std::vector<uint8_t> ImageBuffer::to8bit() const {
    std::vector<uint8_t> result(m_data.size());
    
    // Convert 16-bit to 8-bit (simple downscaling)
    for (size_t i = 0; i < m_data.size(); ++i) {
        result[i] = static_cast<uint8_t>(m_data[i] >> 8);
    }
    
    return result;
}

void ImageBuffer::clear() {
    std::fill(m_data.begin(), m_data.end(), 0);
}

} // namespace zraw
