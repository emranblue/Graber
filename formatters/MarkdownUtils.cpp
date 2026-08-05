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
