#include "XMPHandler.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDateTime>
#include <iostream>

namespace zraw {

XMPHandler::XMPHandler() {
}

QString XMPHandler::getXMPPath(const QString& rawFilePath) {
    QFileInfo fileInfo(rawFilePath);
    QString basePath = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName();
    return basePath + ".xmp";
}

bool XMPHandler::xmpExists(const QString& rawFilePath) {
    QString xmpPath = getXMPPath(rawFilePath);
    return QFile::exists(xmpPath);
}

XMPHandler::Adjustments XMPHandler::loadAdjustments(const QString& rawFilePath) {
    Adjustments adjustments;
    QString xmpPath = getXMPPath(rawFilePath);
    
    if (!QFile::exists(xmpPath)) {
        std::cout << "No XMP file found, using defaults" << std::endl;
        return adjustments;
    }
    
    QFile file(xmpPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Failed to open XMP file: " << xmpPath.toStdString() << std::endl;
        return adjustments;
    }
    
    QString content = file.readAll();
    file.close();
    
    return parseXMP(content);
}

bool XMPHandler::saveAdjustments(const QString& rawFilePath, const Adjustments& adjustments) {
    QString xmpPath = getXMPPath(rawFilePath);
    
    QFile file(xmpPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to create XMP file: " << xmpPath.toStdString() << std::endl;
        return false;
    }
    
    QString xmpContent = generateXMP(adjustments);
    QTextStream out(&file);
    out << xmpContent;
    file.close();
    
    std::cout << "Saved adjustments to XMP: " << xmpPath.toStdString() << std::endl;
    return true;
}

QString XMPHandler::generateXMP(const Adjustments& adjustments) {
    QString xmp;
    
    // XMP packet header (required by XMP spec)
    xmp += "<?xpacket begin=\"\xEF\xBB\xBF\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n";
    
    // XML and XMP metadata
    xmp += "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"ZRaw Developer 0.1.0\">\n";
    xmp += "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n";
    xmp += "    <rdf:Description rdf:about=\"\"\n";
    xmp += "        xmlns:zraw=\"https://github.com/jasonzondor/zraw-developer/\"\n";
    xmp += "        xmlns:crs=\"http://ns.adobe.com/camera-raw-settings/1.0/\"\n";
    xmp += "        xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\"\n";
    
    // ZRaw adjustments
    xmp += "        zraw:Exposure=\"" + QString::number(adjustments.exposure, 'f', 2) + "\"\n";
    xmp += "        zraw:Contrast=\"" + QString::number(adjustments.contrast, 'f', 2) + "\"\n";
    xmp += "        zraw:Sharpness=\"" + QString::number(adjustments.sharpness, 'f', 2) + "\"\n";
    xmp += "        zraw:Temperature=\"" + QString::number(adjustments.temperature, 'f', 1) + "\"\n";
    xmp += "        zraw:Tint=\"" + QString::number(adjustments.tint, 'f', 1) + "\"\n";
    xmp += "        zraw:Highlights=\"" + QString::number(adjustments.highlights, 'f', 1) + "\"\n";
    xmp += "        zraw:Shadows=\"" + QString::number(adjustments.shadows, 'f', 1) + "\"\n";
    xmp += "        zraw:Vibrance=\"" + QString::number(adjustments.vibrance, 'f', 1) + "\"\n";
    xmp += "        zraw:Saturation=\"" + QString::number(adjustments.saturation, 'f', 1) + "\"\n";
    xmp += "        zraw:HighlightContrast=\"" + QString::number(adjustments.highlightContrast, 'f', 1) + "\"\n";
    xmp += "        zraw:MidtoneContrast=\"" + QString::number(adjustments.midtoneContrast, 'f', 1) + "\"\n";
    xmp += "        zraw:ShadowContrast=\"" + QString::number(adjustments.shadowContrast, 'f', 1) + "\"\n";
    
    // Camera Raw compatibility
    xmp += "        crs:Exposure2012=\"" + QString::number(adjustments.exposure, 'f', 2) + "\"\n";
    xmp += "        crs:Contrast2012=\"" + QString::number(adjustments.contrast * 100, 'f', 0) + "\"\n";
    xmp += "        crs:Sharpness=\"" + QString::number(adjustments.sharpness * 50, 'f', 0) + "\"\n";
    xmp += "        crs:Temperature=\"" + QString::number(adjustments.temperature, 'f', 0) + "\"\n";
    xmp += "        crs:Tint=\"" + QString::number(adjustments.tint, 'f', 0) + "\"\n";
    xmp += "        crs:Highlights2012=\"" + QString::number(adjustments.highlights, 'f', 0) + "\"\n";
    xmp += "        crs:Shadows2012=\"" + QString::number(adjustments.shadows, 'f', 0) + "\"\n";
    xmp += "        crs:Vibrance=\"" + QString::number(adjustments.vibrance, 'f', 0) + "\"\n";
    xmp += "        crs:Saturation=\"" + QString::number(adjustments.saturation, 'f', 0) + "\"\n";
    
    // XMP metadata
    xmp += "        xmp:ModifyDate=\"" + QDateTime::currentDateTime().toString(Qt::ISODate) + "\"\n";
    xmp += "        xmp:CreatorTool=\"ZRaw Developer 0.1.0\"/>\n";
    
    xmp += "  </rdf:RDF>\n";
    xmp += "</x:xmpmeta>\n";
    
    // XMP packet trailer (required by XMP spec)
    xmp += "<?xpacket end=\"w\"?>\n";
    
    return xmp;
}

XMPHandler::Adjustments XMPHandler::parseXMP(const QString& xmpContent) {
    Adjustments adjustments;
    
    QXmlStreamReader reader(xmpContent);
    
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isStartElement() && reader.name() == QString("Description")) {
            QXmlStreamAttributes attrs = reader.attributes();
            
            // Try ZRaw namespace first
            if (attrs.hasAttribute("zraw:Exposure")) {
                adjustments.exposure = attrs.value("zraw:Exposure").toFloat();
            }
            if (attrs.hasAttribute("zraw:Contrast")) {
                adjustments.contrast = attrs.value("zraw:Contrast").toFloat();
            }
            if (attrs.hasAttribute("zraw:Sharpness")) {
                adjustments.sharpness = attrs.value("zraw:Sharpness").toFloat();
            }
            if (attrs.hasAttribute("zraw:Temperature")) {
                adjustments.temperature = attrs.value("zraw:Temperature").toFloat();
            }
            if (attrs.hasAttribute("zraw:Tint")) {
                adjustments.tint = attrs.value("zraw:Tint").toFloat();
            }
            if (attrs.hasAttribute("zraw:Highlights")) {
                adjustments.highlights = attrs.value("zraw:Highlights").toFloat();
            }
            if (attrs.hasAttribute("zraw:Shadows")) {
                adjustments.shadows = attrs.value("zraw:Shadows").toFloat();
            }
            if (attrs.hasAttribute("zraw:Vibrance")) {
                adjustments.vibrance = attrs.value("zraw:Vibrance").toFloat();
            }
            if (attrs.hasAttribute("zraw:Saturation")) {
                adjustments.saturation = attrs.value("zraw:Saturation").toFloat();
            }
            if (attrs.hasAttribute("zraw:HighlightContrast")) {
                adjustments.highlightContrast = attrs.value("zraw:HighlightContrast").toFloat();
            }
            if (attrs.hasAttribute("zraw:MidtoneContrast")) {
                adjustments.midtoneContrast = attrs.value("zraw:MidtoneContrast").toFloat();
            }
            if (attrs.hasAttribute("zraw:ShadowContrast")) {
                adjustments.shadowContrast = attrs.value("zraw:ShadowContrast").toFloat();
            }
            
            // Fallback to Camera Raw format
            if (adjustments.exposure == 0.0f && attrs.hasAttribute("crs:Exposure2012")) {
                adjustments.exposure = attrs.value("crs:Exposure2012").toFloat();
            }
            if (adjustments.contrast == 0.0f && attrs.hasAttribute("crs:Contrast2012")) {
                adjustments.contrast = attrs.value("crs:Contrast2012").toFloat() / 100.0f;
            }
            if (adjustments.sharpness == 0.0f && attrs.hasAttribute("crs:Sharpness")) {
                adjustments.sharpness = attrs.value("crs:Sharpness").toFloat() / 50.0f;
            }
            if (adjustments.temperature == 0.0f && attrs.hasAttribute("crs:Temperature")) {
                adjustments.temperature = attrs.value("crs:Temperature").toFloat();
            }
            if (adjustments.tint == 0.0f && attrs.hasAttribute("crs:Tint")) {
                adjustments.tint = attrs.value("crs:Tint").toFloat();
            }
            if (adjustments.highlights == 0.0f && attrs.hasAttribute("crs:Highlights2012")) {
                adjustments.highlights = attrs.value("crs:Highlights2012").toFloat();
            }
            if (adjustments.shadows == 0.0f && attrs.hasAttribute("crs:Shadows2012")) {
                adjustments.shadows = attrs.value("crs:Shadows2012").toFloat();
            }
            if (adjustments.vibrance == 0.0f && attrs.hasAttribute("crs:Vibrance")) {
                adjustments.vibrance = attrs.value("crs:Vibrance").toFloat();
            }
            if (adjustments.saturation == 0.0f && attrs.hasAttribute("crs:Saturation")) {
                adjustments.saturation = attrs.value("crs:Saturation").toFloat();
            }
            
            std::cout << "Loaded adjustments from XMP:" << std::endl;
            std::cout << "  Exposure: " << adjustments.exposure << std::endl;
            std::cout << "  Contrast: " << adjustments.contrast << std::endl;
            std::cout << "  Sharpness: " << adjustments.sharpness << std::endl;
            std::cout << "  Temperature: " << adjustments.temperature << std::endl;
            std::cout << "  Tint: " << adjustments.tint << std::endl;
            std::cout << "  Highlights: " << adjustments.highlights << std::endl;
            std::cout << "  Shadows: " << adjustments.shadows << std::endl;
            std::cout << "  Vibrance: " << adjustments.vibrance << std::endl;
            std::cout << "  Saturation: " << adjustments.saturation << std::endl;
            
            break;
        }
    }
    
    if (reader.hasError()) {
        std::cerr << "XMP parse error: " << reader.errorString().toStdString() << std::endl;
    }
    
    return adjustments;
}

} // namespace zraw
