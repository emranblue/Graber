#include "normalizer.h"
#include "../../utilities/file_handler.h"
#include "../../utilities/logger.h"
#include "../../utilities/slug_generator.h"
#include "../../core/section_definitions.h"
#include <QRegularExpression>
#include <QStringList>

void DocumentNormalizer::normalizeMarkdownFile(const QString &file_path) {
    QString content = FileHandler::readFile(file_path);
    if (content.isEmpty()) {
        return;
    }
    
    // Remove existing TOC block if present
    int toc_start = content.indexOf("<!-- TOC_START -->");
    int toc_end = content.indexOf("<!-- TOC_END -->");
    QString clean_content = content;
    
    if (toc_start != -1 && toc_end != -1) {
        QString pre_toc = content.left(toc_start);
        QString post_toc = content.mid(toc_end + QString("<!-- TOC_END -->").length());
        clean_content = pre_toc + post_toc;
    }
    
    QStringList lines = clean_content.split('\n');
    QStringList output_lines;
    
    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression md_regex("^(#{1,3})\\s+(?!\\*\\*\\*)(.*?)$");
    
    for (const QString &line : lines) {
        QString trimmed_line = line.trimmed();
        
        QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        QRegularExpressionMatch md_match = md_regex.match(trimmed_line);
        
        if (h2_match.hasMatch()) {
            QString title = h2_match.captured(2).trimmed();
            QString slug = QString::fromStdString(SlugGenerator::generate(title));
            QString section = SectionDefinitions::detectSectionFromTitle(title);
            
            QString rewritten_line = normalizeHeading(title, section);
            output_lines.append(rewritten_line);
        } else if (md_match.hasMatch()) {
            int level = md_match.captured(1).length();
            QString title = md_match.captured(2).trimmed();
            QString section = SectionDefinitions::detectSectionFromTitle(title);
            
            if (level == 2) {
                QString rewritten_line = normalizeHeading(title, section);
                output_lines.append(rewritten_line);
            } else {
                output_lines.append(line);
            }
        } else {
            output_lines.append(line);
        }
    }
    
    FileHandler::writeFile(file_path, output_lines.join('\n'));
    Logger::info(QString("Normalized: %1").arg(file_path));
}

QString DocumentNormalizer::normalizeHeading(const QString &title, const QString &section) {
    QString slug = QString::fromStdString(SlugGenerator::generate(title));
    return QString("<h2 id=\"%1\" data-section=\"%2\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">%3</h2>")
        .arg(slug, section, title);
}

void DocumentNormalizer::restoreStateFromFile(const QString &file_path) {
    QString content = FileHandler::readFile(file_path);
    QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9\u0985-\u09b90-9]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|\u099c\u09be\u09a8\u09c1[a-z]*)).*$");
    
    QString last_found_date = "";
    QStringList lines = content.split('\n');
    
    for (const QString &line : lines) {
        QRegularExpressionMatch match = date_regex.match(line.trimmed());
        if (match.hasMatch()) {
            last_found_date = match.captured(1).trimmed();
        }
    }
    
    ApplicationState::instance().setLastDate(last_found_date);
}
