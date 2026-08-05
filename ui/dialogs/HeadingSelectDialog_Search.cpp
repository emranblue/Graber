#include "HeadingSelectDialog.h"
#include <QVector>
#include "Utils.h"

int HeadingSelectDialog::calculate_relevance(const NoteItem &item, const QStringList &keywords) {
    if (keywords.isEmpty()) return 0;
    
    int score = 0;
    
    for (const QString &kw : keywords) {
        if (kw.isEmpty()) continue;
        
        // Exact match in title (highest priority)
        if (item.title.compare(kw, Qt::CaseInsensitive) == 0) {
            score += 100;
        }
        // Title starts with keyword
        else if (item.title.startsWith(kw, Qt::CaseInsensitive)) {
            score += 80;
        }
        // Keyword found in title
        else if (item.title.contains(kw, Qt::CaseInsensitive)) {
            score += 50;
        }
        
        // Fuzzy match in title
        if (fuzzy_match(item.title, kw)) {
            score += 30;
        }
        
        // Slug matches
        if (item.slug.contains(kw, Qt::CaseInsensitive)) {
            score += 20;
        }
        
        // Section matches
        if (item.section.contains(kw, Qt::CaseInsensitive)) {
            score += 10;
        }
    }
    
    return score;
}

bool HeadingSelectDialog::fuzzy_match(const QString &text, const QString &pattern) {
    if (pattern.isEmpty()) return true;
    if (text.isEmpty()) return false;
    
    int text_idx = 0;
    int pattern_idx = 0;
    
    while (text_idx < text.length() && pattern_idx < pattern.length()) {
        if (text[text_idx].toLower() == pattern[pattern_idx].toLower()) {
            pattern_idx++;
        }
        text_idx++;
    }
    
    return pattern_idx == pattern.length();
}

QString HeadingSelectDialog::highlight_text(const QString &src, const QStringList &keywords) {
    if (keywords.isEmpty() || src.isEmpty()) return src;
    
    int n = src.length();
    QVector<bool> is_matched(n, false);
    
    for (const QString &kw : keywords) {
        if (kw.isEmpty()) continue;
        int kw_len = kw.length();
        int pos = 0;
        while ((pos = src.indexOf(kw, pos, Qt::CaseInsensitive)) != -1) {
            for (int i = 0; i < kw_len; ++i) {
                is_matched[pos + i] = true;
            }
            pos += kw_len;
        }
    }
    
    QString result;
    bool in_highlight = false;
    for (int i = 0; i < n; ++i) {
        if (is_matched[i]) {
            if (!in_highlight) {
                result += "<span style=\"background-color: #f1c40f; color: #2c3e50; font-weight: bold;\">";
                in_highlight = true;
            }
        } else {
            if (in_highlight) {
                result += "</span>";
                in_highlight = false;
            }
        }
        
        QChar c = src[i];
        if (c == '<') result += "&lt;";
        else if (c == '>') result += "&gt;";
        else if (c == '&') result += "&amp;";
        else result += c;
    }
    if (in_highlight) {
        result += "</span>";
    }
    return result;
}
