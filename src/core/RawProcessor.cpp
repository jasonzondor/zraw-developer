#include "RawProcessor.h"
#include <libraw/libraw.h>
#include <iostream>
#include <cstring>

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
    
    // Configure processing parameters
    libraw_output_params_t& params = m_libraw->imgdata.params;
    
    // Detect X-Trans sensor (Fujifilm)
    bool isXTrans = (m_libraw->imgdata.idata.filters == 9);
    
    if (isXTrans) {
        // X-Trans sensors: Use 3-pass algorithm (best quality for X-Trans)
        // Options: 0=linear, 1=VNG, 2=PPG, 3=AHD, 4=DCB, 11=DHT, 12=AAHD
        params.user_qual = 11;  // DHT (11) or AAHD (12) work best for X-Trans
        std::cout << "Detected X-Trans sensor, using DHT demosaicing" << std::endl;
    } else {
        // Bayer sensors: Use AHD (Adaptive Homogeneity-Directed)
        params.user_qual = 3;  // AHD is excellent for Bayer
        std::cout << "Detected Bayer sensor, using AHD demosaicing" << std::endl;
    }
    
    // Use camera white balance - this is the most accurate
    params.use_camera_wb = 1;
    
    // Output 16-bit for better quality
    params.output_bps = 16;
    
    // Use sRGB color space (can be changed to Adobe RGB if needed)
    params.output_color = 1;  // 1=sRGB, 2=Adobe RGB
    
    // No automatic brightness adjustment (we'll do this in GPU pipeline)
    params.no_auto_bright = 1;
    
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
    
    uint16_t* dest = m_buffer->data();
    
    // Check if output is 8-bit or 16-bit
    if (image->bits == 8) {
        // Convert 8-bit to 16-bit
        const uint8_t* src = image->data;
        size_t pixelCount = static_cast<size_t>(width) * height * channels;
        for (size_t i = 0; i < pixelCount; ++i) {
            dest[i] = static_cast<uint16_t>(src[i]) << 8;
        }
    } else {
        // Copy 16-bit directly
        const uint16_t* src = reinterpret_cast<const uint16_t*>(image->data);
        size_t pixelCount = static_cast<size_t>(width) * height * channels;
        std::memcpy(dest, src, pixelCount * sizeof(uint16_t));
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

float RawProcessor::getCameraWBTemperature() const {
    // Get camera white balance multipliers
    // cam_mul[0] = red, cam_mul[1] = green, cam_mul[2] = blue, cam_mul[3] = green2
    float r_mul = m_libraw->imgdata.color.cam_mul[0];
    float g_mul = m_libraw->imgdata.color.cam_mul[1];
    float b_mul = m_libraw->imgdata.color.cam_mul[2];
    
    // Avoid division by zero
    if (g_mul < 0.001f || r_mul < 0.001f || b_mul < 0.001f) {
        return 5500.0f;  // Default daylight
    }
    
    // Calculate red/blue ratio (normalized by green)
    float r_ratio = r_mul / g_mul;
    float b_ratio = b_mul / g_mul;
    
    // Approximate color temperature from red/blue ratio
    // This is a simplified approximation based on Planckian locus
    // Higher red ratio = warmer (lower Kelvin), higher blue ratio = cooler (higher Kelvin)
    float ratio = r_ratio / b_ratio;
    
    // Empirical formula to convert ratio to Kelvin (approximate)
    // Based on typical camera WB behavior
    float kelvin;
    if (ratio > 1.0f) {
        // Warm light (tungsten to daylight)
        kelvin = 2000.0f + (ratio - 1.0f) * 3500.0f;
    } else {
        // Cool light (daylight to shade)
        kelvin = 5500.0f + (1.0f - ratio) * 4500.0f;
    }
    
    // Clamp to reasonable range
    if (kelvin < 2000.0f) kelvin = 2000.0f;
    if (kelvin > 10000.0f) kelvin = 10000.0f;
    
    return kelvin;
}

void RawProcessor::setError(const std::string& error) {
    m_lastError = error;
    std::cerr << "RawProcessor error: " << error << std::endl;
}

} // namespace zraw
