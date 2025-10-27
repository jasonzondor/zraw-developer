#include "CLIHandler.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

namespace zraw {

CLIHandler::CLIHandler() : m_helpRequested(false) {
    setupParser();
}

void CLIHandler::setupParser() {
    m_parser.setApplicationDescription(
        "ZRaw Developer - GPU-accelerated RAW photo editor for Linux\n\n"
        "Usage modes:\n"
        "  GUI mode:      zraw-developer [input.raw]\n"
        "  Headless mode: zraw-developer --headless -i input.raw -o output.tiff [options]"
    );
    
    m_parser.addHelpOption();
    m_parser.addVersionOption();
    
    // Input/Output
    m_parser.addOption(QCommandLineOption(
        {"i", "input"},
        "Input RAW file path",
        "file"
    ));
    
    m_parser.addOption(QCommandLineOption(
        {"o", "output"},
        "Output file path (required in headless mode)",
        "file"
    ));
    
    // Mode
    m_parser.addOption(QCommandLineOption(
        "headless",
        "Run in headless mode (no GUI, process and exit)"
    ));
    
    // Adjustments
    m_parser.addOption(QCommandLineOption(
        {"e", "exposure"},
        "Exposure adjustment in stops (-3.0 to +3.0)",
        "value",
        "0.0"
    ));
    
    m_parser.addOption(QCommandLineOption(
        {"c", "contrast"},
        "Contrast adjustment (-1.0 to +1.0)",
        "value",
        "0.0"
    ));
    
    m_parser.addOption(QCommandLineOption(
        {"s", "sharpness"},
        "Sharpness adjustment (0.0 to 2.0)",
        "value",
        "0.0"
    ));
    
    // Output format
    m_parser.addOption(QCommandLineOption(
        {"f", "format"},
        "Output format: tiff, jpeg, png",
        "format",
        "tiff"
    ));
    
    m_parser.addOption(QCommandLineOption(
        {"q", "quality"},
        "JPEG quality (1-100, default: 95)",
        "value",
        "95"
    ));
    
    // Positional argument for input file
    m_parser.addPositionalArgument("input", "Input RAW file (optional)", "[input.raw]");
}

bool CLIHandler::parse(const QStringList& arguments) {
    if (!m_parser.parse(arguments)) {
        return false;
    }
    
    if (m_parser.isSet("help")) {
        m_helpRequested = true;
        return true;
    }
    
    // Get input file (from option or positional argument)
    if (m_parser.isSet("input")) {
        m_options.inputFile = m_parser.value("input");
    } else {
        QStringList positional = m_parser.positionalArguments();
        if (!positional.isEmpty()) {
            m_options.inputFile = positional.first();
        }
    }
    
    // Headless mode
    m_options.headless = m_parser.isSet("headless");
    
    // In headless mode, output file is required
    if (m_options.headless) {
        if (!m_parser.isSet("output")) {
            qCritical() << "Error: --output is required in headless mode";
            return false;
        }
        m_options.outputFile = m_parser.value("output");
        
        if (m_options.inputFile.isEmpty()) {
            qCritical() << "Error: --input is required in headless mode";
            return false;
        }
    } else {
        // GUI mode - output is optional
        if (m_parser.isSet("output")) {
            m_options.outputFile = m_parser.value("output");
        }
    }
    
    // Parse adjustments
    bool ok;
    m_options.exposure = m_parser.value("exposure").toFloat(&ok);
    if (!ok || m_options.exposure < -3.0f || m_options.exposure > 3.0f) {
        qCritical() << "Error: Exposure must be between -3.0 and +3.0";
        return false;
    }
    
    m_options.contrast = m_parser.value("contrast").toFloat(&ok);
    if (!ok || m_options.contrast < -1.0f || m_options.contrast > 1.0f) {
        qCritical() << "Error: Contrast must be between -1.0 and +1.0";
        return false;
    }
    
    m_options.sharpness = m_parser.value("sharpness").toFloat(&ok);
    if (!ok || m_options.sharpness < 0.0f || m_options.sharpness > 2.0f) {
        qCritical() << "Error: Sharpness must be between 0.0 and 2.0";
        return false;
    }
    
    // Output format
    m_options.format = m_parser.value("format").toLower();
    if (m_options.format != "tiff" && m_options.format != "jpeg" && 
        m_options.format != "jpg" && m_options.format != "png") {
        qCritical() << "Error: Format must be tiff, jpeg, or png";
        return false;
    }
    if (m_options.format == "jpg") {
        m_options.format = "jpeg";
    }
    
    // JPEG quality
    m_options.quality = m_parser.value("quality").toInt(&ok);
    if (!ok || m_options.quality < 1 || m_options.quality > 100) {
        qCritical() << "Error: Quality must be between 1 and 100";
        return false;
    }
    
    return true;
}

QString CLIHandler::helpText() const {
    return m_parser.helpText();
}

} // namespace zraw
