#include "HeadingSelectDialog.h"
#include <QLabel>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QVector>
#include <algorithm>
#include "Utils.h"
#include "MarkdownUtils.h"

void HeadingSelectDialog::populate_list(const QString &search_text) {
    debugLog(QString("populate_list (dynamic): search_text='%1', all_headings_.size()=%2").arg(search_text, QString::number(all_headings_.size())));
    list_widget_->clear();
    
    QString normalized_search = search_text.normalized(QString::NormalizationForm_C);
    QStringList keywords = normalized_search.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    
    // Add "Append to End"
    bool show_append_to_end = true;
    if (!keywords.isEmpty()) {
        QString append_option_text = "(শেষে নতুন করে যোগ করুন / Append to End)";
        for (const QString &kw : keywords) {
            if (!append_option_text.contains(kw, Qt::CaseInsensitive) && !fuzzy_match(append_option_text, kw)) {
                show_append_to_end = false;
                break;
            }
        }
    }
    
    if (show_append_to_end) {
        QListWidgetItem *item = new QListWidgetItem(list_widget_);
        item->setData(Qt::UserRole, "");
        item->setData(Qt::UserRole + 1, "(শেষে নতুন করে যোগ করুন / Append to End)");
        
        QLabel *label = new QLabel();
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
        label->setTextFormat(Qt::RichText);
        
        QString disp_text = "(শেষে নতুন করে যোগ করুন / Append to End)";
        if (!keywords.isEmpty()) {
            disp_text = highlight_text(disp_text, keywords);
        }
        label->setText(QString("<b>%1</b>").arg(disp_text));
        label->setStyleSheet("padding: 6px; color: #2f3640;");
        
        list_widget_->addItem(item);
        list_widget_->setItemWidget(item, label);
        
        label->adjustSize();
        item->setSizeHint(QSize(0, qMax(label->sizeHint().height(), 36)));
    }
    
    // Collect results with relevance scores
    QVector<SearchResult> results;
    
    for (int i = 0; i < all_headings_.size(); ++i) {
        const auto &heading = all_headings_.at(i);
        bool matches = true;
        
        if (!keywords.isEmpty()) {
            for (const QString &kw : keywords) {
                bool kw_found = heading.title.contains(kw, Qt::CaseInsensitive) || 
                                heading.slug.contains(kw, Qt::CaseInsensitive) ||
                                heading.section.contains(kw, Qt::CaseInsensitive) ||
                                fuzzy_match(heading.title, kw);
                if (!kw_found) {
                    matches = false;
                    break;
                }
            }
        }
        
        if (matches) {
            int relevance = calculate_relevance(heading, keywords);
            results.append({i, relevance, &heading});
        }
    }
    
    // Sort by relevance score (highest first)
    std::sort(results.begin(), results.end(), 
        [](const SearchResult &a, const SearchResult &b) {
            return a.relevance_score > b.relevance_score;
        });
    
    // Populate sorted results
    for (const auto &result : results) {
        const NoteItem *heading = result.item;
        const QString display_id = (result.index >= 0 && result.index < display_ids_.size())
            ? display_ids_.at(result.index) : QString();
        QListWidgetItem *item = new QListWidgetItem(list_widget_);
        item->setData(Qt::UserRole, heading->slug);
        item->setData(Qt::UserRole + 1, heading->title);
        
        QLabel *label = new QLabel();
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
        label->setTextFormat(Qt::RichText);
        
        QString disp_html = "";
        
        if (heading->type == "heading") {
            QString title_part = heading->title;
            QString slug_part = heading->slug;
            QString section_part = heading->section;
            
            if (!keywords.isEmpty()) {
                title_part = highlight_text(title_part, keywords);
                slug_part = highlight_text(slug_part, keywords);
            }
            
            // Add relevance indicator if searching
            QString relevance_indicator = "";
            if (!keywords.isEmpty() && result.relevance_score > 0) {
                int stars = qMin(5, (result.relevance_score / 20) + 1);
                relevance_indicator = QString(" <span style=\"color: #f39c12;\">%1</span>").arg(QString("★").repeated(stars));
            }
            
            disp_html = QString("<span style=\"font-size: 14px; font-weight: bold; color: #e74c3c;\">%1</span>%2 <span style=\"font-size: 11px; color: #7f8c8d;\">(id: %3) [%4]</span>")
                        .arg(title_part, relevance_indicator, display_id, section_part.toUpper());
        } else {
            QString title_part = heading->title;
            if (!keywords.isEmpty()) {
                title_part = highlight_text(title_part, keywords);
            }
            
            disp_html = QString("<span style=\"padding-left: 15px; font-size: 13px; color: #2980b9;\">↳ %1</span> <span style=\"font-size: 11px; color: #7f8c8d;\">(id: %2)</span>")
                        .arg(title_part, display_id);
        }
        
        label->setText(disp_html);
        label->setStyleSheet("padding: 6px;");
        
        list_widget_->addItem(item);
        list_widget_->setItemWidget(item, label);
        
        label->adjustSize();
        item->setSizeHint(QSize(0, qMax(label->sizeHint().height(), 36)));
    }
    
    // Auto select first item
    if (list_widget_->count() > 0) {
        list_widget_->setCurrentRow(0);
    }
}

