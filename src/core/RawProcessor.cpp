#include "RawProcessor.h"
#include <libraw/libraw.h>
#include <iostream>
#include <cstring>

namespace zraw {

RawProcessor::RawProcessor()
    : m_libraw(std::make_unique<LibRaw>()),
      m_buffer(std::make_shared<ImageBuffer>()) {
}

void RawProcessor::setLinearOutput(bool enable) {
    m_linearOutput = enable;
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
    
    // Use camera white balance as the starting point
    // LibRaw applies this correctly in camera RGB space before color matrix conversion
    params.use_camera_wb = 1;
    
    // Don't set user_mul - let LibRaw use camera WB
    // We'll apply fine-tune adjustments in the GPU shader on top of this
    
    // Output 16-bit for better quality
    params.output_bps = 16;
    
    // No automatic brightness adjustment (we'll do this in GPU pipeline)
    params.no_auto_bright = 1;
    
    // Configure color space and gamma based on requested output mode
    if (m_linearOutput) {
        // Linear sRGB: sRGB primaries, but linear gamma (scene-referred)
        params.output_color = 1;   // 1 = sRGB
        params.gamm[0] = 1.0f;     // gamma power (linear)
        params.gamm[1] = 1.0f;     // toe/offset
        std::cout << "RawProcessor: using linear sRGB output" << std::endl;
    } else {
        // Display-ready sRGB from LibRaw (gamma-encoded)
        params.output_color = 1;   // 1 = sRGB
        // Leave LibRaw's default gamma (sRGB-like)
        std::cout << "RawProcessor: using LibRaw sRGB output" << std::endl;
    }
    
    // Debug: print color matrix info
    std::cout << "Camera: " << m_libraw->imgdata.idata.make << " " << m_libraw->imgdata.idata.model << std::endl;
    std::cout << "Color matrix available: " << (m_libraw->imgdata.color.cam_xyz[0][0] != 0.0f ? "yes" : "no") << std::endl;
    
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
    // Note: WBCT_Coeffs contains preset temperatures, not necessarily the as-shot WB
    // For accurate temperature display, we need to calculate from the actual multipliers
    // that were applied (cam_mul), similar to how Darktable does it
    
    // Get the actual white balance multipliers that were applied
    // cam_mul[0] = red, cam_mul[1] = green, cam_mul[2] = blue, cam_mul[3] = green2
    float r_mul = m_libraw->imgdata.color.cam_mul[0];
    float g_mul = m_libraw->imgdata.color.cam_mul[1];
    float b_mul = m_libraw->imgdata.color.cam_mul[2];
    
    std::cout << "Camera WB multipliers: R=" << r_mul << " G=" << g_mul << " B=" << b_mul << std::endl;
    
    // Avoid division by zero
    if (g_mul < 0.001f || r_mul < 0.001f || b_mul < 0.001f) {
        return 5500.0f;  // Default daylight
    }
    
    // Normalize by green (standard practice)
    float r_norm = r_mul / g_mul;
    float b_norm = b_mul / g_mul;
    
    // Calculate color temperature using Robertson's method (simplified)
    // This is based on the red/blue ratio on the Planckian locus
    float ratio = r_norm / b_norm;
    
    // Improved empirical formula based on typical camera behavior
    // This approximates the Planckian locus more accurately
    float kelvin;
    if (ratio >= 1.0f) {
        // Warmer temperatures (red > blue)
        // Use logarithmic scale for better accuracy
        kelvin = 3000.0f + 2500.0f * std::log(ratio);
    } else {
        // Cooler temperatures (blue > red)
        kelvin = 7000.0f - 2000.0f * std::log(1.0f / ratio);
    }
    
    // Clamp to reasonable photographic range
    if (kelvin < 2000.0f) kelvin = 2000.0f;
    if (kelvin > 10000.0f) kelvin = 10000.0f;
    
    std::cout << "Calculated temperature from multipliers: " << kelvin << "K (ratio=" << ratio << ")" << std::endl;
    return kelvin;
}

float RawProcessor::getCameraWBTint() const {
    // Get camera white balance multipliers
    // cam_mul[0] = red, cam_mul[1] = green, cam_mul[2] = blue, cam_mul[3] = green2
    float r_mul = m_libraw->imgdata.color.cam_mul[0];
    float g_mul = m_libraw->imgdata.color.cam_mul[1];
    float b_mul = m_libraw->imgdata.color.cam_mul[2];
    
    // Avoid division by zero
    if (g_mul < 0.001f || r_mul < 0.001f || b_mul < 0.001f) {
        return 0.0f;  // Neutral tint
    }
    
    // Normalize by green
    float r_norm = r_mul / g_mul;
    float b_norm = b_mul / g_mul;
    
    // Tint represents deviation from the Planckian locus (green-magenta axis)
    // Darktable uses: tint = (green - expected_green) / expected_green
    // where expected_green is based on the Planckian locus for the given temperature
    
    // For a neutral tint, green should be 1.0 (since we normalized by green)
    // The expected green for the Planckian locus is approximately the geometric mean of R and B
    float expected_green_ratio = std::sqrt(r_norm * b_norm);
    
    // Calculate tint as deviation from expected
    // Darktable's tint scale is typically around 0.5 to 1.5, with 1.0 being neutral
    float tint = 1.0f / expected_green_ratio;
    
    std::cout << "Calculated tint: " << tint << " (r_norm=" << r_norm << ", b_norm=" << b_norm << ")" << std::endl;
    
    return tint;
}

void RawProcessor::getCameraWBMultipliers(float multipliers[4]) const {
    if (!m_libraw) {
        multipliers[0] = multipliers[1] = multipliers[2] = multipliers[3] = 1.0f;
        return;
    }
    
    // Get camera white balance multipliers from LibRaw
    // cam_mul[0] = red, cam_mul[1] = green, cam_mul[2] = blue, cam_mul[3] = green2
    multipliers[0] = m_libraw->imgdata.color.cam_mul[0];
    multipliers[1] = m_libraw->imgdata.color.cam_mul[1];
    multipliers[2] = m_libraw->imgdata.color.cam_mul[2];
    multipliers[3] = m_libraw->imgdata.color.cam_mul[3];
    
    // Normalize by green (standard practice in RAW processing)
    // Use G1 for normalization (G2 is often 0 for many cameras)
    float g_norm = multipliers[1];
    if (g_norm > 0.001f) {
        multipliers[0] /= g_norm;
        multipliers[1] /= g_norm;  // Will be 1.0
        multipliers[2] /= g_norm;
        if (multipliers[3] > 0.001f) {
            multipliers[3] /= g_norm;
        }
    }
    
    std::cout << "Camera WB multipliers (normalized by G): R=" << multipliers[0] 
              << " G=" << multipliers[1] << " B=" << multipliers[2] 
              << " G2=" << multipliers[3] << std::endl;
}

void RawProcessor::setError(const std::string& error) {
    m_lastError = error;
    std::cerr << "RawProcessor error: " << error << std::endl;
}

} // namespace zraw
