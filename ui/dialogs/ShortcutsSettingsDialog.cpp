#include "ShortcutsSettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>
#include <QPushButton>

ShortcutsSettingsDialog::ShortcutsSettingsDialog(QList<ShortcutConfig> &configs,
                                                   bool &globalHotkeysEnabled,
                                                   bool globalHotkeysSupported,
                                                   bool &diagramPanelEnabled,
                                                   QWidget *parent)
    : QDialog(parent), configs_(configs), global_hotkeys_enabled_(globalHotkeysEnabled),
      diagram_panel_enabled_(diagramPanelEnabled) {
    setWindowTitle("কীবোর্ড শর্টকাট সেটিংস (Keyboard Shortcut Settings)");
    setMinimumSize(520, 560);

    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(12);
    main_layout->setContentsMargins(15, 15, 15, 15);

    QLabel *title = new QLabel("শর্টকাটসমূহ পরিবর্তন করুন (Edit Shortcuts):");
    title->setStyleSheet("font-weight: bold; font-size: 16px; color: #192a56; border: none; background: transparent;");
    main_layout->addWidget(title);

    // --- Global hotkeys toggle ---
    QFrame *global_frame = new QFrame();
    global_frame->setStyleSheet("QFrame { background-color: white; border: 1px solid #dcdde1; border-radius: 6px; padding: 6px; }");
    QVBoxLayout *global_layout = new QVBoxLayout(global_frame);

    global_hotkeys_checkbox_ = new QCheckBox("সিস্টেম-ওয়াইড শর্টকাট চালু রাখুন (Work even when another window is focused)");
    global_hotkeys_checkbox_->setStyleSheet("font-weight: bold; color: #2f3640;");
    global_hotkeys_checkbox_->setChecked(global_hotkeys_enabled_);
    global_layout->addWidget(global_hotkeys_checkbox_);

    if (!globalHotkeysSupported) {
        global_hotkeys_checkbox_->setEnabled(false);
        global_hotkeys_checkbox_->setChecked(false);
        QLabel *unsupported_label = new QLabel(
            "এই সিস্টেমে সিস্টেম-ওয়াইড শর্টকাট সমর্থিত নয় (সম্ভবত Wayland)। "
            "শর্টকাটগুলো শুধু অ্যাপ উইন্ডো ফোকাসে থাকলে কাজ করবে।\n"
            "(Global shortcuts aren't supported in this session.)");
        unsupported_label->setWordWrap(true);
        unsupported_label->setStyleSheet("font-size: 11px; color: #e84118; border: none; background: transparent;");
        global_layout->addWidget(unsupported_label);
    } else {
        QLabel *desc_label = new QLabel(
            "চালু থাকলে, নিচের সক্রিয় শর্টকাটগুলো যেকোনো অ্যাপে থাকলেও কাজ করবে।\n"
            "(When on, enabled shortcuts below work system-wide.)");
        desc_label->setWordWrap(true);
        desc_label->setStyleSheet("font-size: 11px; color: #7f8c8d; border: none; background: transparent;");
        global_layout->addWidget(desc_label);
    }
    main_layout->addWidget(global_frame);

    // --- Diagram panel toggle ---
    QFrame *diagram_frame = new QFrame();
    diagram_frame->setStyleSheet("QFrame { background-color: white; border: 1px solid #dcdde1; border-radius: 6px; padding: 6px; }");
    QVBoxLayout *diagram_frame_layout = new QVBoxLayout(diagram_frame);

    diagram_panel_checkbox_ = new QCheckBox("ডায়াগ্রাম প্যানেল দেখান (Show Insert Diagram panel)");
    diagram_panel_checkbox_->setStyleSheet("font-weight: bold; color: #2f3640;");
    diagram_panel_checkbox_->setChecked(diagram_panel_enabled_);
    diagram_frame_layout->addWidget(diagram_panel_checkbox_);

    QLabel *diagram_desc_label = new QLabel(
        "চালু থাকলে মূল উইন্ডোতে ডায়াগ্রাম সারি দেখা যাবে।\n"
        "(When on, the Insert Diagram row is shown on the main window.)");
    diagram_desc_label->setWordWrap(true);
    diagram_desc_label->setStyleSheet("font-size: 11px; color: #7f8c8d; border: none; background: transparent;");
    diagram_frame_layout->addWidget(diagram_desc_label);
    main_layout->addWidget(diagram_frame);

    // --- Per-shortcut rows ---
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
        item_layout->setSpacing(10);

        // Enable switch
        QCheckBox *en = new QCheckBox();
        en->setToolTip("এই শর্টকাট সক্রিয়/নিষ্ক্রিয় (Enable / disable this shortcut)");
        en->setChecked(cfg.enabled);
        en->setStyleSheet("QCheckBox { spacing: 0; }");
        item_layout->addWidget(en);
        enable_checks_.append(en);

        QVBoxLayout *text_layout = new QVBoxLayout();
        QLabel *name_label = new QLabel(cfg.name_bangla);
        name_label->setStyleSheet("font-weight: bold; font-size: 14px; color: #2f3640; border: none; background: transparent;");
        QLabel *desc_label = new QLabel(cfg.name_english);
        desc_label->setStyleSheet("font-size: 11px; color: #7f8c8d; border: none; background: transparent;");
        text_layout->addWidget(name_label);
        text_layout->addWidget(desc_label);
        item_layout->addLayout(text_layout, 1);

        QKeySequenceEdit *key_edit = new QKeySequenceEdit(cfg.current_key);
        key_edit->setStyleSheet("QKeySequenceEdit { border: 1px solid #dcdde1; padding: 4px; background-color: #f5f6fa; color: black; min-width: 140px; }");
        key_edit->setEnabled(cfg.enabled);
        item_layout->addWidget(key_edit);
        edits_.append(key_edit);

        // Grey out key edit when disabled
        connect(en, &QCheckBox::toggled, key_edit, &QWidget::setEnabled);

        scroll_layout->addWidget(item_frame);
    }

    scroll_layout->addStretch(1);
    scroll->setWidget(scroll_widget);
    main_layout->addWidget(scroll, 1);

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
        enable_checks_[i]->setChecked(true);
    }
}

void ShortcutsSettingsDialog::on_save() {
    for (int i = 0; i < configs_.size(); ++i) {
        configs_[i].current_key = edits_[i]->keySequence();
        configs_[i].enabled = enable_checks_[i]->isChecked();
    }
    global_hotkeys_enabled_ = global_hotkeys_checkbox_->isChecked();
    diagram_panel_enabled_ = diagram_panel_checkbox_->isChecked();
    accept();
}
