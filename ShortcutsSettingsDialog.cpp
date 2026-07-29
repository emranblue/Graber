#include "ShortcutsSettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QPushButton>

ShortcutsSettingsDialog::ShortcutsSettingsDialog(QList<ShortcutConfig> &configs, QWidget *parent) 
    : QDialog(parent), configs_(configs) {
    setWindowTitle("কীবোর্ড শর্টকাট সেটিংস (Keyboard Shortcut Settings)");
    setMinimumSize(450, 500);
    
    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(12);
    main_layout->setContentsMargins(15, 15, 15, 15);
    
    QLabel *title = new QLabel("শর্টকাটসমূহ পরিবর্তন করুন (Edit Shortcuts):");
    title->setStyleSheet("font-weight: bold; font-size: 16px; color: #192a56; border: none; background: transparent;");
    main_layout->addWidget(title);
    
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    QWidget *scroll_widget = new QWidget();
    scroll_widget->setStyleSheet("background-color: transparent;");
    QVBoxLayout *scroll_layout = new QVBoxLayout(scroll_widget);
    scroll_layout->setSpacing(8);
    scroll_layout->setContentsMargins(0, 0, 0, 0);
    
    for (int i = 0; i < configs_.size(); ++i) {
        const auto &cfg = configs_[i];
        
        QFrame *item_frame = new QFrame();
        item_frame->setStyleSheet("QFrame { background-color: white; border: 1px solid #dcdde1; border-radius: 6px; padding: 6px; }");
        QHBoxLayout *item_layout = new QHBoxLayout(item_frame);
        item_layout->setContentsMargins(8, 8, 8, 8);
        
        QVBoxLayout *text_layout = new QVBoxLayout();
        QLabel *name_label = new QLabel(cfg.name_bangla);
        name_label->setStyleSheet("font-weight: bold; font-size: 16px; color: #2f3640; border: none; background: transparent;");
        QLabel *desc_label = new QLabel(cfg.name_english);
        desc_label->setStyleSheet("font-size: 11px; color: #7f8c8d; border: none; background: transparent;");
        text_layout->addWidget(name_label);
        text_layout->addWidget(desc_label);
        item_layout->addLayout(text_layout, 1);
        
        QKeySequenceEdit *key_edit = new QKeySequenceEdit(cfg.current_key);
        key_edit->setStyleSheet("QKeySequenceEdit { border: 1px solid #dcdde1; padding: 4px; background-color: #f5f6fa; color: black; min-width: 150px; }");
        item_layout->addWidget(key_edit);
        
        edits_.append(key_edit);
        
        scroll_layout->addWidget(item_frame);
    }
    
    scroll->setWidget(scroll_widget);
    main_layout->addWidget(scroll, 1);
    
    // Buttons
    QHBoxLayout *btn_layout = new QHBoxLayout();
    QPushButton *reset_btn = new QPushButton("ডিফল্ট রিসেট (Reset Defaults)");
    reset_btn->setStyleSheet("QPushButton { background-color: #e84118; color: white; } QPushButton:hover { background-color: #c23616; }");
    
    QPushButton *save_btn = new QPushButton("সংরক্ষণ করুন (Save)");
    save_btn->setStyleSheet("QPushButton { background-color: #44bd32; color: white; } QPushButton:hover { background-color: #2b8a1a; }");
    
    QPushButton *cancel_btn = new QPushButton("বাতিল (Cancel)");
    cancel_btn->setStyleSheet("QPushButton { background-color: #718093; color: white; } QPushButton:hover { background-color: #57606f; }");
    
    btn_layout->addWidget(reset_btn);
    btn_layout->addStretch();
    btn_layout->addWidget(save_btn);
    btn_layout->addWidget(cancel_btn);
    
    main_layout->addLayout(btn_layout);
    
    connect(reset_btn, &QPushButton::clicked, this, &ShortcutsSettingsDialog::on_reset);
    connect(save_btn, &QPushButton::clicked, this, &ShortcutsSettingsDialog::on_save);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
}

void ShortcutsSettingsDialog::on_reset() {
    for (int i = 0; i < configs_.size(); ++i) {
        edits_[i]->setKeySequence(QKeySequence(configs_[i].default_key));
    }
}

void ShortcutsSettingsDialog::on_save() {
    for (int i = 0; i < configs_.size(); ++i) {
        configs_[i].current_key = edits_[i]->keySequence();
    }
    accept();
}
