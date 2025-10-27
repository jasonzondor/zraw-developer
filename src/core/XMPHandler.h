#pragma once

#include <QString>
#include <QMap>

namespace zraw {

/**
 * XMP sidecar file handler for non-destructive editing
 * Stores adjustment values in XMP format compatible with photo management apps
 */
class XMPHandler {
public:
    struct Adjustments {
        float exposure = 0.0f;      // -3.0 to +3.0 EV
        float contrast = 0.0f;      // -1.0 to +1.0
        float sharpness = 0.0f;     // 0.0 to 2.0
        
        // Future adjustments
        float temperature = 0.0f;   // White balance
        float tint = 0.0f;
        float saturation = 0.0f;
        float highlights = 0.0f;
        float shadows = 0.0f;
    };
    
    XMPHandler();
    
    /**
     * Load adjustments from XMP sidecar file
     * @param rawFilePath Path to the RAW file (XMP will be .xmp extension)
     * @return Adjustments structure with loaded values
     */
    Adjustments loadAdjustments(const QString& rawFilePath);
    
    /**
     * Save adjustments to XMP sidecar file
     * @param rawFilePath Path to the RAW file
     * @param adjustments Adjustment values to save
     * @return true on success
     */
    bool saveAdjustments(const QString& rawFilePath, const Adjustments& adjustments);
    
    /**
     * Check if XMP sidecar exists for RAW file
     */
    bool xmpExists(const QString& rawFilePath);
    
    /**
     * Get XMP file path for a RAW file
     */
    static QString getXMPPath(const QString& rawFilePath);

private:
    QString generateXMP(const Adjustments& adjustments);
    Adjustments parseXMP(const QString& xmpContent);
    
    QString m_lastError;
};

} // namespace zraw
