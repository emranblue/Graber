#include "HeadingSelectDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QVector>
#include <QTimer>
#include <QScrollBar>
#include "Utils.h"
#include "MarkdownUtils.h"

HeadingSelectDialog::HeadingSelectDialog(const QList<NoteItem> &all_headings, const QString &current_slug, QWidget *parent)
    : QDialog(parent), all_headings_(all_headings),
      display_ids_(MarkdownUtils::compute_display_ids(all_headings)), selected_slug_(current_slug) {
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

