#pragma once

#include <QString>
#include <QCommandLineParser>
#include <memory>

namespace zraw {

/**
 * Command-line interface handler
 * Supports both GUI and headless processing modes
 */
class CLIHandler {
public:
    struct Options {
        QString inputFile;
        QString outputFile;
        bool headless = false;
        
        // Adjustments
        float exposure = 0.0f;      // -3.0 to +3.0
        float contrast = 0.0f;      // -1.0 to +1.0
        float sharpness = 0.0f;     // 0.0 to 2.0
        
        // Output format
        QString format = "tiff";    // tiff, jpeg, png
        int quality = 95;           // For JPEG (1-100)
    };
    
    CLIHandler();
    
    /**
     * Parse command line arguments
     */
    bool parse(const QStringList& arguments);
    
    /**
     * Get parsed options
     */
    const Options& options() const { return m_options; }
    
    /**
     * Check if help was requested
     */
    bool helpRequested() const { return m_helpRequested; }
    
    /**
     * Get help text
     */
    QString helpText() const;

private:
    Options m_options;
    bool m_helpRequested;
    QCommandLineParser m_parser;
    
    void setupParser();
};

} // namespace zraw
