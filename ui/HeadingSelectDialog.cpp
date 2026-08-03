#include "HeadingSelectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QVector>
#include <QTimer>
#include <QScrollBar>
#include "../utils/Utils.h"

HeadingSelectDialog::HeadingSelectDialog(const QList<NoteItem> &all_headings, const QString &current_slug, QWidget *parent)
    : QDialog(parent), all_headings_(all_headings), selected_slug_(current_slug) {
    setWindowTitle("টার্গেট শিরোনাম নির্বাচন (Select Target Heading)");
    
    setStyleSheet(
        "QDialog { background-color: #f5f6fa; font-family: 'Segoe UI', 'Kalpurush'; color: #2f3640; }"
        "QLabel { color: #2f3640; font-size: 14px; background: transparent; }"
        "QLineEdit { background: white; color: black; padding: 8px; border: 1px solid #dcdde1; border-radius: 4px; font-size: 14px; }"
        "QListWidget { background: white; border: 1px solid #dcdde1; border-radius: 6px; padding: 5px; color: black; }"
        "QPushButton { background-color: #487eb0; color: white; border-radius: 5px; padding: 10px; font-weight: bold; border: none; min-width: 80px; }"
        "QPushButton:hover { background-color: #40739e; }"
        "QPushButton:disabled { background-color: #dcdde1; color: #7f8c8d; }"
    );

    if (parent) {
        resize(parent->size());
    } else {
        resize(540, 660);
    }
    
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(10);
    main_layout->setContentsMargins(12, 12, 12, 12);
    
    // Search input with status info
    QHBoxLayout *search_layout = new QHBoxLayout();
    search_edit_ = new QLineEdit(this);
    search_edit_->setPlaceholderText("খুঁজুন... (Type to search...) - Fuzzy matching enabled");
    search_edit_->installEventFilter(this);
    search_edit_->setFocus();
    search_layout->addWidget(search_edit_);
    main_layout->addLayout(search_layout);
    
    list_widget_ = new QListWidget(this);
    list_widget_->setWordWrap(true);
    main_layout->addWidget(list_widget_, 1);
    
    QHBoxLayout *btn_layout = new QHBoxLayout();
    QPushButton *select_btn = new QPushButton("নির্বাচন করুন (Select)", this);
    select_btn->setStyleSheet("QPushButton { background-color: #44bd32; color: white; } QPushButton:hover { background-color: #2b8a1a; }");
    
    QPushButton *cancel_btn = new QPushButton("বাতিল (Cancel)", this);
    cancel_btn->setStyleSheet("QPushButton { background-color: #718093; color: white; } QPushButton:hover { background-color: #57606f; }");
    
    btn_layout->addStretch();
    btn_layout->addWidget(select_btn);
    btn_layout->addWidget(cancel_btn);
    main_layout->addLayout(btn_layout);
    
    // Setup debounce timer for smooth real-time search
    debounce_timer_ = new QTimer(this);
    debounce_timer_->setSingleShot(true);
    connect(debounce_timer_, &QTimer::timeout, this, &HeadingSelectDialog::on_debounce_timer);
    
    connect(search_edit_, &QLineEdit::textChanged, this, [this](const QString &text) {
        pending_search_text_ = text;
        debounce_timer_->stop();
        debounce_timer_->start(100); // Wait 100ms before filtering (smooth UX)
    });
    connect(list_widget_, &QListWidget::itemDoubleClicked, this, &HeadingSelectDialog::on_item_double_clicked);
    connect(select_btn, &QPushButton::clicked, this, &HeadingSelectDialog::on_select_clicked);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
    
    populate_list("");
    
    // Select the current item initially
    bool found_current = false;
    for (int i = 0; i < list_widget_->count(); ++i) {
        QListWidgetItem *item = list_widget_->item(i);
        if (item->data(Qt::UserRole).toString() == selected_slug_) {
            list_widget_->setCurrentItem(item);
            found_current = true;
            break;
        }
    }
    if (!found_current && list_widget_->count() > 0) {
        list_widget_->setCurrentRow(0);
    }
}

bool HeadingSelectDialog::eventFilter(QObject *obj, QEvent *event) {
    if (obj == search_edit_ && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up) {
            QCoreApplication::sendEvent(list_widget_, keyEvent);
            return true;
        } else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            on_select_clicked();
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void HeadingSelectDialog::on_debounce_timer() {
    on_search_text_changed(pending_search_text_);
}

void HeadingSelectDialog::on_search_text_changed(const QString &text) {
    debugLog(QString("on_search_text_changed (dynamic): text='%1'").arg(text));
    populate_list(text);
}

void HeadingSelectDialog::on_item_double_clicked(QListWidgetItem *item) {
    if (item) {
        selected_slug_ = item->data(Qt::UserRole).toString();
        selected_title_ = item->data(Qt::UserRole + 1).toString();
        accept();
    }
}

void HeadingSelectDialog::on_select_clicked() {
    QListWidgetItem *item = list_widget_->currentItem();
    if (item) {
        selected_slug_ = item->data(Qt::UserRole).toString();
        selected_title_ = item->data(Qt::UserRole + 1).toString();
        accept();
    } else {
        reject();
    }
}

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
                        .arg(title_part, relevance_indicator, slug_part, section_part.toUpper());
        } else {
            QString title_part = heading->title;
            QString slug_part = heading->slug;
            if (!keywords.isEmpty()) {
                title_part = highlight_text(title_part, keywords);
                slug_part = highlight_text(slug_part, keywords);
            }
            
            disp_html = QString("<span style=\"padding-left: 15px; font-size: 13px; color: #2980b9;\">↳ %1</span> <span style=\"font-size: 11px; color: #7f8c8d;\">(id: %2)</span>")
                        .arg(title_part, slug_part);
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
