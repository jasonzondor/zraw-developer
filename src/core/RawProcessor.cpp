#include "RawProcessor.h"
#include <libraw/libraw.h>
#include <iostream>

namespace zraw {

RawProcessor::RawProcessor()
    : m_libraw(std::make_unique<LibRaw>()),
      m_buffer(std::make_shared<ImageBuffer>()) {
}

RawProcessor::~RawProcessor() {
}

bool RawProcessor::loadRaw(const std::string& filepath) {
    // Open raw file
    int ret = m_libraw->open_file(filepath.c_str());
    if (ret != LIBRAW_SUCCESS) {
        setError(std::string("Failed to open file: ") + libraw_strerror(ret));
        return false;
    }
    
    return true;
}

bool RawProcessor::processToRGB() {
    // Unpack raw data
    int ret = m_libraw->unpack();
    if (ret != LIBRAW_SUCCESS) {
        setError(std::string("Failed to unpack: ") + libraw_strerror(ret));
        return false;
    }
    
    // Process to RGB
    ret = m_libraw->dcraw_process();
    if (ret != LIBRAW_SUCCESS) {
        setError(std::string("Failed to process: ") + libraw_strerror(ret));
        return false;
    }
    
    // Get processed image
    libraw_processed_image_t* image = m_libraw->dcraw_make_mem_image(&ret);
    if (!image) {
        setError(std::string("Failed to create image: ") + libraw_strerror(ret));
        return false;
    }
    
    // Copy to our buffer
    int width = image->width;
    int height = image->height;
    int channels = image->colors;
    
    m_buffer->allocate(width, height, channels);
    
    // Convert 8-bit to 16-bit for processing
    uint16_t* dest = m_buffer->data();
    const uint8_t* src = image->data;
    size_t pixelCount = static_cast<size_t>(width) * height * channels;
    
    for (size_t i = 0; i < pixelCount; ++i) {
        dest[i] = static_cast<uint16_t>(src[i]) << 8;
    }
    
    // Free LibRaw image
    LibRaw::dcraw_clear_mem(image);
    
    return true;
}

int RawProcessor::width() const {
    return m_libraw->imgdata.sizes.width;
}

int RawProcessor::height() const {
    return m_libraw->imgdata.sizes.height;
}

std::string RawProcessor::cameraModel() const {
    return std::string(m_libraw->imgdata.idata.model);
}

float RawProcessor::exposureTime() const {
    return m_libraw->imgdata.other.shutter;
}

float RawProcessor::aperture() const {
    return m_libraw->imgdata.other.aperture;
}

int RawProcessor::iso() const {
    return static_cast<int>(m_libraw->imgdata.other.iso_speed);
}

void RawProcessor::setError(const std::string& error) {
    m_lastError = error;
    std::cerr << "RawProcessor error: " << error << std::endl;
}

} // namespace zraw
