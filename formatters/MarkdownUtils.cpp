#include "MarkdownUtils.h"
#include <cctype>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

namespace MarkdownUtils {

std::string generate_slug(const QString &text) {
    std::string result;
    bool last_was_hyphen = true; // start with true to avoid leading hyphen
    
    std::string utf8_str = text.trimmed().toStdString();
    for (size_t i = 0; i < utf8_str.length(); ) {
        unsigned char c = utf8_str[i];
        if (c < 128) {
            // ASCII character
            if (std::isalnum(c)) {
                result += std::tolower(c);
                last_was_hyphen = false;
                i++;
            } else {
                if (!last_was_hyphen) {
                    result += '-';
                    last_was_hyphen = true;
                }
                i++;
            }
        } else {
            // UTF-8 multi-byte character (keep it as is)
            size_t len = 1;
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;
            
            if (i + len <= utf8_str.length()) {
                result.append(utf8_str.c_str() + i, len);
                last_was_hyphen = false;
                i += len;
            } else {
                i++; // invalid UTF-8 byte
            }
        }
    }
    
    // Trim trailing hyphen
    while (!result.empty() && result.back() == '-') {
        result.pop_back();
    }
    return result;
}

void save_tree_file(const QString &file_path, const QList<NoteItem> &items) {
    QString tree_path = file_path;
    tree_path.replace(".md", ".tree");
    
    QFile file(tree_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out << "Note Structure Tree\n";
    out << "===================\n\n";

    for (const NoteItem &item : items) {
        if (item.type == "heading") {
            out << QString("- [%1] %2 (id: %3)\n").arg(item.section.toUpper(), item.title, item.slug);
        } else if (item.type == "subheading") {
            out << QString("  └── %1 (id: %2)\n").arg(item.title, item.slug);
        }
    }
    file.close();
}

QString restore_state_from_file(const QString &file_path) {
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }
    
    QTextStream in(&file);
    QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
    
    QString last_found_date = "";
    
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QRegularExpressionMatch match = date_regex.match(line);
        if (match.hasMatch()) {
            last_found_date = match.captured(1).trimmed();
        }
    }
    file.close();
    
    return last_found_date;
}

bool get_heading_bounds(const QString &content, const QString &slug, int &start_pos, int &end_pos, bool &is_html) {
    QString html_pattern = QString("<h2[^>]*id=\"%1\"[^>]*>").arg(QRegularExpression::escape(slug));
    QRegularExpression html_rx(html_pattern, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch html_match = html_rx.match(content);
    
    if (html_match.hasMatch()) {
        is_html = true;
        start_pos = html_match.capturedStart();
        
        int next_h2 = content.indexOf("<h2", html_match.capturedEnd(), Qt::CaseInsensitive);
        int next_h3 = content.indexOf("<h3", html_match.capturedEnd(), Qt::CaseInsensitive);
        
        QRegularExpression next_md_rx("^#{1,3}\\s+", QRegularExpression::MultilineOption);
        QRegularExpressionMatch next_md_match = next_md_rx.match(content, html_match.capturedEnd());
        int next_md = next_md_match.hasMatch() ? next_md_match.capturedStart() : -1;
        
        int min_pos = content.length();
        if (next_h2 != -1 && next_h2 < min_pos) min_pos = next_h2;
        if (next_h3 != -1 && next_h3 < min_pos) min_pos = next_h3;
        if (next_md != -1 && next_md < min_pos) min_pos = next_md;
        
        end_pos = min_pos;
        return true;
    }
    
    QStringList lines = content.split('\n');
    int char_counter = 0;
    QRegularExpression md_rx("^(#{2})\\s+(.*?)$");
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        int line_len_with_nl = line.length() + 1;
        QRegularExpressionMatch md_match = md_rx.match(line.trimmed());
        
        if (md_match.hasMatch()) {
            QString title = md_match.captured(2).trimmed();
            QString lslug = QString::fromStdString(generate_slug(title));
            if (lslug == slug) {
                is_html = false;
                start_pos = char_counter;
                
                int next_heading_pos = -1;
                int current_char_pos = char_counter + line_len_with_nl;
                for (int j = i + 1; j < lines.size(); ++j) {
                    QString next_line = lines[j];
                    if (md_rx.match(next_line.trimmed()).hasMatch() || next_line.trimmed().startsWith("<h2")) {
                        next_heading_pos = current_char_pos;
                        break;
                    }
                    current_char_pos += next_line.length() + 1;
                }
                
                if (next_heading_pos != -1) {
                    end_pos = next_heading_pos;
                } else {
                    end_pos = content.length();
                }
                return true;
            }
        }
        char_counter += line_len_with_nl;
    }
    
    return false;
}

bool get_subheading_insert_pos(const QString &content, const QString &slug, int &insert_pos) {
    QString html_pattern = QString("<h3[^>]*id=\"%1\"[^>]*>").arg(QRegularExpression::escape(slug));
    QRegularExpression html_rx(html_pattern, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch html_match = html_rx.match(content);
    
    int start_search = -1;
    if (html_match.hasMatch()) {
        start_search = html_match.capturedEnd();
    } else {
        QStringList lines = content.split('\n');
        int char_counter = 0;
        QRegularExpression md_sub_rx("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QRegularExpressionMatch md_match = md_sub_rx.match(line.trimmed());
            if (md_match.hasMatch()) {
                QString title = md_match.captured(2).trimmed();
                QString lslug = QString::fromStdString(generate_slug(title));
                if (lslug == slug) {
                    start_search = char_counter + line.length() + 1;
                    break;
                }
            }
            char_counter += line.length() + 1;
        }
    }

    if (start_search == -1) {
        return false;
    }

    int next_h3 = content.indexOf("<h3", start_search, Qt::CaseInsensitive);
    int next_h2 = content.indexOf("<h2", start_search, Qt::CaseInsensitive);
    
    QRegularExpression next_md_rx("^#{2,3}\\s+", QRegularExpression::MultilineOption);
    QRegularExpressionMatch next_md_match = next_md_rx.match(content, start_search);
    int next_md = next_md_match.hasMatch() ? next_md_match.capturedStart() : -1;

    int min_pos = content.length();
    if (next_h3 != -1 && next_h3 < min_pos) min_pos = next_h3;
    if (next_h2 != -1 && next_h2 < min_pos) min_pos = next_h2;
    if (next_md != -1 && next_md < min_pos) min_pos = next_md;

    insert_pos = min_pos;
    return true;
}

bool get_subheading_bounds(const QString &content, const QString &slug, int &start_pos, int &end_pos) {
    QString html_pattern = QString("<h3[^>]*id=\"%1\"[^>]*>").arg(QRegularExpression::escape(slug));
    QRegularExpression html_rx(html_pattern, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch html_match = html_rx.match(content);
    
    if (html_match.hasMatch()) {
        start_pos = html_match.capturedStart();
    } else {
        QStringList lines = content.split('\n');
        int char_counter = 0;
        QRegularExpression md_sub_rx("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
        bool found = false;
        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QRegularExpressionMatch md_match = md_sub_rx.match(line.trimmed());
            if (md_match.hasMatch()) {
                QString title = md_match.captured(2).trimmed();
                QString lslug = QString::fromStdString(generate_slug(title));
                if (lslug == slug) {
                    start_pos = char_counter;
                    found = true;
                    break;
                }
            }
            char_counter += line.length() + 1;
        }
        if (!found) return false;
    }

    return get_subheading_insert_pos(content, slug, end_pos);
}

QStringList compute_display_ids(const QList<NoteItem> &all_headings) {
    QStringList ids;
    ids.reserve(all_headings.size());

    int main_counter = 0;
    int sub_counter = 0;

    for (const NoteItem &item : all_headings) {
        if (item.type == "heading") {
            main_counter++;
            sub_counter = 0;
            ids.append(QString::number(main_counter));
        } else {
            sub_counter++;
            ids.append(QString("%1.%2").arg(main_counter).arg(sub_counter));
        }
    }

    return ids;
}

} // namespace MarkdownUtils
