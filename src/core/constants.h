#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants {
    // UI Colors
    const QString COLOR_PRIMARY = "#487eb0";
    const QString COLOR_PRIMARY_HOVER = "#40739e";
    const QString COLOR_SUCCESS = "#44bd32";
    const QString COLOR_DANGER = "#e84118";
    const QString COLOR_WARNING = "#f39c12";
    const QString COLOR_INFO = "#0097e6";
    const QString COLOR_TEXT = "#2f3640";
    const QString COLOR_BORDER = "#dcdde1";
    const QString COLOR_BG = "#f5f6fa";
    
    // Heading Colors
    const QString COLOR_HEADING_H2 = "#e74c3c";
    const QString COLOR_HEADING_H3 = "#2980b9";
    
    // Styles
    const QString STYLE_H2 = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
    const QString STYLE_H3 = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
    
    // Directories
    const QString DIR_IMAGES = "images";
    const QString DIR_DELETED = "deleted";
    
    // File Extensions
    const QString EXT_MARKDOWN = ".md";
    const QString EXT_TREE = ".tree";
    const QString EXT_INI = ".ini";
}

#endif // CONSTANTS_H
