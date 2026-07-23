#include "toc_generator.h"
#include "../../utilities/file_handler.h"
#include "../../utilities/logger.h"
#include "../../utilities/slug_generator.h"
#include "../../core/section_definitions.h"
#include <QRegularExpression>
#include <QStringList>

void TOCGenerator::updateTOC(const QString &file_path) {
    QString content = FileHandler::readFile(file_path);
    if (content.isEmpty()) {
        return;
    }
    
    // Remove existing TOC blocks
    QString clean_content = content;
    while (true) {
        int toc_start = clean_content.indexOf("<!-- TOC_START -->");
        int toc_end = clean_content.indexOf("<!-- TOC_END -->");
        if (toc_start != -1 && toc_end != -1 && toc_end > toc_start) {
            QString pre_toc = clean_content.left(toc_start);
            QString post_toc = clean_content.mid(toc_end + QString("<!-- TOC_END -->").length());
            clean_content = pre_toc + post_toc;
        } else {
            break;
        }
    }
    
    // Parse headings
    QStringList lines = clean_content.split('\n');
    QList<HeadingInfo> headings;
    QString current_date = "";
    
    QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9\u0985-\u09b90-9]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|\u099c\u09be\u09a8\u09c1[a-z]*)).*$");
    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    
    int heading_counter = 0;
    for (const QString &line : lines) {
        QString trimmed_line = line.trimmed();
        
        QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
        QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
        
        if (date_match.hasMatch()) {
            current_date = date_match.captured(1).trimmed();
        } else if (h2_match.hasMatch()) {
            heading_counter++;
            QString title = h2_match.captured(2).trimmed();
            QString slug = QString::fromStdString(SlugGenerator::generate(title));
            QString section = "others";
            
            QRegularExpressionMatch section_match = section_attr_regex.match(h2_match.captured(1));
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
            }
            
            HeadingInfo info;
            info.index = heading_counter;
            info.title = title;
            info.slug = slug;
            info.date = current_date;
            info.is_html = true;
            info.level = 2;
            info.section = section;
            headings.append(info);
        } else if (h3_match.hasMatch()) {
            QString title = h3_match.captured(2).trimmed();
            QString slug = QString::fromStdString(SlugGenerator::generate(title));
            
            HeadingInfo info;
            info.index = 0;
            info.title = title;
            info.slug = slug;
            info.date = "";
            info.is_html = true;
            info.level = 3;
            info.section = "others";
            headings.append(info);
        }
    }
    
    // Generate TOC
    QString toc_block = generateTOCBlock(headings);
    
    // Combine and write
    QString final_content = "";
    QString body_content = clean_content.trimmed();
    
    if (!toc_block.isEmpty()) {
        final_content = toc_block + "\n\n" + body_content;
    } else {
        final_content = body_content;
    }
    
    FileHandler::writeFile(file_path, final_content);
    Logger::info(QString("TOC updated: %1").arg(file_path));
}

QString TOCGenerator::generateTOCBlock(const QList<HeadingInfo> &headings) {
    if (headings.isEmpty()) {
        return "";
    }
    
    QString toc_block = "<!-- TOC_START -->\n";
    toc_block += "## \u09b8\u09c2\u099a\u09bf\u09aa\u09a4\u09cd\u09b0 (Table of Contents)\n\n";
    
    auto sections_list = SectionDefinitions::getDefaultSections();
    
    for (const auto &sec : sections_list) {
        QList<HeadingInfo> sec_headings;
        for (const HeadingInfo &info : headings) {
            if (info.section == sec.key && info.level == 2) {
                sec_headings.append(info);
            }
        }
        
        if (!sec_headings.isEmpty()) {
            toc_block += QString("### %1 (%2)\n").arg(sec.bangla, sec.english);
            toc_block += "| \u09aa\u09c3\u09b7\u09cd\u09a0\u09be (Page) | \u09a4\u09be\u09b0\u09bf\u0996 (Date) | \u0986\u0987\u09a1\u09bf (ID) | \u09b6\u09bf\u09b0\u09cb\u09a8\u09be\u09ae (Chapter/Topic) |\n";
            toc_block += "| :---: | :---: | :---: | :--- |\n";
            
            for (const HeadingInfo &info : sec_headings) {
                QString date_str = info.date.isEmpty() ? "---" : info.date;
                toc_block += QString("| **%1** | %2 | `%3` | [%4](#%5) |\n")
                    .arg(QString::number(info.index), date_str, info.slug, info.title, info.slug);
            }
            toc_block += "\n";
        }
    }
    
    toc_block += "---\n";
    toc_block += "<!-- TOC_END -->\n";
    
    return toc_block;
}

void TOCGenerator::parseNoteStructure(const QString &file_path, QList<NoteItem> &items) {
    items.clear();
    QString content = FileHandler::readFile(file_path);
    
    // Strip TOC
    int toc_start = content.indexOf("<!-- TOC_START -->");
    int toc_end = content.indexOf("<!-- TOC_END -->");
    QString clean_content = content;
    if (toc_start != -1 && toc_end != -1) {
        QString pre_toc = content.left(toc_start);
        QString post_toc = content.mid(toc_end + QString("<!-- TOC_END -->").length());
        clean_content = pre_toc + post_toc;
    }
    
    QStringList lines = clean_content.split('\n');
    QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
    
    QString current_h2_slug = "";
    
    for (const QString &line : lines) {
        QString trimmed_line = line.trimmed();
        
        QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
        QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
        
        if (h2_match.hasMatch()) {
            QString title = h2_match.captured(2).trimmed();
            QString slug = QString::fromStdString(SlugGenerator::generate(title));
            QString section = "others";
            
            QRegularExpressionMatch section_match = section_attr_regex.match(h2_match.captured(1));
            if (section_match.hasMatch()) {
                section = section_match.captured(1);
            }
            
            NoteItem item = {title, slug, "heading", section, ""};
            items.append(item);
            current_h2_slug = slug;
        } else if (h3_match.hasMatch()) {
            QString title = h3_match.captured(2).trimmed();
            QString slug = QString::fromStdString(SlugGenerator::generate(title));
            
            NoteItem item = {title, slug, "subheading", "others", current_h2_slug};
            items.append(item);
        }
    }
}

bool TOCGenerator::getHeadingBounds(const QString &content, const QString &slug, int &start_pos, int &end_pos, bool &is_html) {
    QString html_pattern = QString("<h2[^>]*id=\"%1\"[^>]*>").arg(QRegularExpression::escape(slug));
    QRegularExpression html_rx(html_pattern, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch html_match = html_rx.match(content);
    
    if (html_match.hasMatch()) {
        is_html = true;
        start_pos = html_match.capturedStart();
        
        int next_h2 = content.indexOf("<h2", html_match.capturedEnd(), Qt::CaseInsensitive);
        int next_h3 = content.indexOf("<h3", html_match.capturedEnd(), Qt::CaseInsensitive);
        
        int min_pos = content.length();
        if (next_h2 != -1 && next_h2 < min_pos) min_pos = next_h2;
        if (next_h3 != -1 && next_h3 < min_pos) min_pos = next_h3;
        
        end_pos = min_pos;
        return true;
    }
    
    return false;
}

bool TOCGenerator::getSubheadingBounds(const QString &content, const QString &slug, int &start_pos, int &end_pos) {
    QString html_pattern = QString("<h3[^>]*id=\"%1\"[^>]*>").arg(QRegularExpression::escape(slug));
    QRegularExpression html_rx(html_pattern, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch html_match = html_rx.match(content);
    
    if (html_match.hasMatch()) {
        start_pos = html_match.capturedStart();
        
        int next_h3 = content.indexOf("<h3", html_match.capturedEnd(), Qt::CaseInsensitive);
        int next_h2 = content.indexOf("<h2", html_match.capturedEnd(), Qt::CaseInsensitive);
        
        int min_pos = content.length();
        if (next_h3 != -1 && next_h3 < min_pos) min_pos = next_h3;
        if (next_h2 != -1 && next_h2 < min_pos) min_pos = next_h2;
        
        end_pos = min_pos;
        return true;
    }
    
    return false;
}
