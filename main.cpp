#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QClipboard>
#include <QGuiApplication>
#include <QImage>
#include <QFrame>
#include <QDialog>
#include <QListWidget>
#include <QCompleter>
#include <QMimeData>
#include <QComboBox>
#include <QLineEdit>
#include <QScrollArea>
#include <QInputDialog>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <string>
#include <fstream>
#include <iostream>
#include <QDesktopServices>
#include <QUrl>
#include <QCloseEvent>
#include <QRegularExpression>
#include <QTextStream>
#include <cctype>

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QShortcut>
#include <QKeySequenceEdit>
#include <QSettings>

inline void debugLog(const QString &msg) {
    QFile file(QDir::homePath() + "/GraberNotes/debug.log");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " - " << msg << "\n";
        file.close();
    }
}

struct ShortcutConfig {
    QString action_id;
    QString name_bangla;
    QString name_english;
    QString default_key;
    QKeySequence current_key;
    QShortcut* shortcut_obj = nullptr;
};

class ShortcutsSettingsDialog : public QDialog {
    Q_OBJECT
public:
    ShortcutsSettingsDialog(QList<ShortcutConfig> &configs, QWidget *parent = nullptr) 
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
            name_label->setStyleSheet("font-weight: bold; font-size: 13px; color: #2f3640; border: none; background: transparent;");
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
    
private slots:
    void on_reset() {
        for (int i = 0; i < configs_.size(); ++i) {
            edits_[i]->setKeySequence(QKeySequence(configs_[i].default_key));
        }
    }
    
    void on_save() {
        for (int i = 0; i < configs_.size(); ++i) {
            configs_[i].current_key = edits_[i]->keySequence();
        }
        accept();
    }
    
private:
    QList<ShortcutConfig> &configs_;
    QList<QKeySequenceEdit*> edits_;
};struct NoteItem {
    QString title;
    QString slug;
    QString type; // "heading" or "subheading"
    QString section;
    QString parent_slug;
};

class HeadingSelectDialog : public QDialog {
    Q_OBJECT
public:
    HeadingSelectDialog(const QList<NoteItem> &all_headings, const QString &current_slug, QWidget *parent = nullptr)
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
        
        search_edit_ = new QLineEdit(this);
        search_edit_->setPlaceholderText("খুঁজুন... (Type to search...)");
        main_layout->addWidget(search_edit_);
        
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
        
        connect(search_edit_, &QLineEdit::textChanged, this, &HeadingSelectDialog::on_search_text_changed);
        connect(list_widget_, &QListWidget::itemDoubleClicked, this, &HeadingSelectDialog::on_item_double_clicked);
        connect(select_btn, &QPushButton::clicked, this, &HeadingSelectDialog::on_select_clicked);
        connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
        
        populate_list("");
        
        for (int i = 0; i < list_widget_->count(); ++i) {
            QListWidgetItem *item = list_widget_->item(i);
            if (item->data(Qt::UserRole).toString() == selected_slug_) {
                list_widget_->setCurrentItem(item);
                break;
            }
        }
        
        search_edit_->setFocus();
    }
    
    QString get_selected_slug() const { return selected_slug_; }
    QString get_selected_title() const { return selected_title_; }
    
private slots:
    void on_search_text_changed(const QString &text) {
        populate_list(text);
    }
    
    void on_item_double_clicked(QListWidgetItem *item) {
        if (item) {
            selected_slug_ = item->data(Qt::UserRole).toString();
            selected_title_ = item->data(Qt::UserRole + 1).toString();
            accept();
        }
    }
    
    void on_select_clicked() {
        QListWidgetItem *item = list_widget_->currentItem();
        if (item) {
            selected_slug_ = item->data(Qt::UserRole).toString();
            selected_title_ = item->data(Qt::UserRole + 1).toString();
            accept();
        } else {
            reject();
        }
    }
    
private:
    void populate_list(const QString &search_text) {
        list_widget_->clear();
        
        // Add "Append to End"
        {
            QListWidgetItem *item = new QListWidgetItem(list_widget_);
            item->setData(Qt::UserRole, "");
            item->setData(Qt::UserRole + 1, "(শেষে নতুন করে যোগ করুন / Append to End)");
            
            QLabel *label = new QLabel();
            QString disp_text = "(শেষে নতুন করে যোগ করুন / Append to End)";
            if (!search_text.isEmpty() && disp_text.contains(search_text, Qt::CaseInsensitive)) {
                disp_text = highlight_text(disp_text, search_text);
            }
            label->setText(QString("<b>%1</b>").arg(disp_text));
            label->setStyleSheet("padding: 6px; color: #2f3640;");
            item->setSizeHint(label->sizeHint());
            list_widget_->addItem(item);
            list_widget_->setItemWidget(item, label);
        }
        
        for (const auto &heading : all_headings_) {
            bool matches = true;
            if (!search_text.isEmpty()) {
                matches = heading.title.contains(search_text, Qt::CaseInsensitive) || 
                          heading.slug.contains(search_text, Qt::CaseInsensitive);
            }
            
            if (matches) {
                QListWidgetItem *item = new QListWidgetItem(list_widget_);
                item->setData(Qt::UserRole, heading.slug);
                item->setData(Qt::UserRole + 1, heading.title);
                
                QLabel *label = new QLabel();
                QString disp_html = "";
                
                if (heading.type == "heading") {
                    QString title_part = heading.title;
                    QString slug_part = heading.slug;
                    QString section_part = heading.section;
                    
                    if (!search_text.isEmpty()) {
                        title_part = highlight_text(title_part, search_text);
                        slug_part = highlight_text(slug_part, search_text);
                    }
                    disp_html = QString("<span style=\"font-size: 14px; font-weight: bold; color: #e74c3c;\">%1</span> <span style=\"font-size: 11px; color: #7f8c8d;\">(id: %2) [%3]</span>")
                                .arg(title_part, slug_part, section_part.toUpper());
                } else {
                    QString title_part = heading.title;
                    QString slug_part = heading.slug;
                    if (!search_text.isEmpty()) {
                        title_part = highlight_text(title_part, search_text);
                        slug_part = highlight_text(slug_part, search_text);
                    }
                    disp_html = QString("<span style=\"padding-left: 15px; font-size: 13px; color: #2980b9;\">↳ %1</span> <span style=\"font-size: 11px; color: #7f8c8d;\">(id: %2)</span>")
                                .arg(title_part, slug_part);
                }
                
                label->setText(disp_html);
                label->setStyleSheet("padding: 4px;");
                item->setSizeHint(label->sizeHint());
                list_widget_->addItem(item);
                list_widget_->setItemWidget(item, label);
            }
        }
    }
    
    QString highlight_text(const QString &src, const QString &search) {
        if (search.isEmpty()) return src;
        
        QString result;
        int last_pos = 0;
        int pos = 0;
        int search_len = search.length();
        
        while ((pos = src.indexOf(search, last_pos, Qt::CaseInsensitive)) != -1) {
            result += src.mid(last_pos, pos - last_pos);
            QString match_text = src.mid(pos, search_len);
            result += QString("<span style=\"background-color: #f1c40f; color: #2c3e50; font-weight: bold;\">%1</span>").arg(match_text);
            last_pos = pos + search_len;
        }
        
        result += src.mid(last_pos);
        return result;
    }
    
    const QList<NoteItem> &all_headings_;
    QString selected_slug_;
    QString selected_title_;
    QLineEdit *search_edit_;
    QListWidget *list_widget_;
};

class ClipboardGrabber : public QWidget {
    Q_OBJECT

public:
    ClipboardGrabber(QWidget *parent = nullptr) : QWidget(parent) {
        // --- Setup ---
        setWindowTitle("ক্লিপবোর্ড গ্র্যাবার");
        setMinimumSize(500, 620);
        resize(540, 660);

        // --- State Variables ---
        is_running_ = false;
        last_simplified_text_ = "";
        last_date_ = "";
        
        // --- Scalability Fix: Use a dedicated notes directory ---
        notes_dir_path_ = QDir::homePath() + QDir::separator() + "GraberNotes";
        QDir dir(notes_dir_path_);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        // --- Initialize Shortcuts ---
        init_shortcut_configs();
        load_settings();
        setup_shortcuts();

        // --- UI Styling ---
        this->setObjectName("MainWindow");

        // --- UI Widgets ---
        status_label_ = new QLabel("অবস্থা: বন্ধ");
        status_label_->setObjectName("status_label");
        status_label_->setAlignment(Qt::AlignCenter);

        last_captured_label_ = new QLabel("শেষ ক্যাপচার: (কিছুই না)");
        last_captured_label_->setObjectName("last_captured_label");
        last_captured_label_->setAlignment(Qt::AlignCenter);
        last_captured_label_->setWordWrap(true);
        last_captured_label_->setMinimumHeight(60);

        start_button_ = new QPushButton("শুরু (Start)");
        start_button_->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");
        stop_button_ = new QPushButton("থামুন (Stop)");
        stop_button_->setEnabled(false);
        stop_button_->setStyleSheet("QPushButton { background-color: #e84118; } QPushButton:hover { background-color: #c23616; }");

        add_image_button_ = new QPushButton("ছবি যুক্ত করুন (Add Image)");
        subject_dropdown_ = new QComboBox();
        add_subject_button_ = new QPushButton("নতুন বিষয় (New Subject)");
        add_subject_button_->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");

        open_file_button_ = new QPushButton("নোট খুলুন (Open Note)");
        open_file_button_->setStyleSheet("QPushButton { background-color: #0097e6; } QPushButton:hover { background-color: #00a8ff; }");
        open_file_button_->setEnabled(false);

        heading_label_ = new QLabel("শিরোনাম (Heading):");
        select_heading_button_ = new QPushButton("(শেষে নতুন করে যোগ করুন / Append to End)");
        select_heading_button_->setObjectName("select_heading_button");
        select_heading_button_->setStyleSheet("QPushButton { background-color: white; color: black; border: 1px solid #dcdde1; text-align: left; padding: 10px; font-weight: normal; border-radius: 4px; } QPushButton:hover { background-color: #f5f6fa; } QPushButton:disabled { background-color: #dcdde1; color: #7f8c8d; }");
        select_heading_button_->setEnabled(false);

        append_to_heading_button_ = new QPushButton("যুক্ত করুন (Append)");
        append_to_heading_button_->setStyleSheet("QPushButton { background-color: #f39c12; } QPushButton:hover { background-color: #e67e22; }");
        append_to_heading_button_->setEnabled(false);

        shift_heading_button_ = new QPushButton("স্থানান্তর (Shift)");
        shift_heading_button_->setStyleSheet("QPushButton { background-color: #3498db; } QPushButton:hover { background-color: #2980b9; }");
        shift_heading_button_->setEnabled(false);

        delete_heading_button_ = new QPushButton("মুছে ফেলুন (Delete)");
        delete_heading_button_->setStyleSheet("QPushButton { background-color: #c0392b; } QPushButton:hover { background-color: #ae2012; }");
        delete_heading_button_->setEnabled(false);

        format_dropdown_ = new QComboBox();
        format_dropdown_->addItem("বুলেট পয়েন্ট (Point)");
        format_dropdown_->addItem("প্রধান শিরোনাম (Heading - Red)");
        format_dropdown_->addItem("উপ-শিরোনাম (Subheading - Blue)");

        section_label_ = new QLabel("বিভাগ (Section):");
        section_dropdown_ = new QComboBox();
        section_dropdown_->addItem("পরিবেশ (Environment)", "environment");
        section_dropdown_->addItem("জ্বালানি (Energy)", "energy");
        section_dropdown_->addItem("অর্থনীতি (Economy)", "economy");
        section_dropdown_->addItem("সংস্কৃতি (Culture)", "culture");
        section_dropdown_->addItem("ভূগোল (Geography)", "geography");
        section_dropdown_->addItem("জনসংখ্যা (Population)", "population");
        section_dropdown_->addItem("আইন ও সংবিধান (Law-Constitution)", "law-constitution");
        section_dropdown_->addItem("রাজনীতি (Politics)", "politics");
        section_dropdown_->addItem("মুক্তিযুদ্ধ (Freedom-Fight)", "freedom-fight");
        section_dropdown_->addItem("কৃষি (Agriculture)", "agriculture");
        section_dropdown_->addItem("ইতিহাস (History)", "history");
        section_dropdown_->addItem("শিক্ষা (Education)", "education");
        section_dropdown_->addItem("স্বাস্থ্য (Health)", "health");
        section_dropdown_->addItem("বিজ্ঞান ও প্রযুক্তি (Science-Tech)", "science-tech");
        section_dropdown_->addItem("পররাষ্ট্রনীতি (Foreign-Policy)", "foreign-policy");
        section_dropdown_->addItem("প্রশাসন (Administration)", "administration");
        section_dropdown_->addItem("অন্যান্য (Others)", "others");

        add_section_button_ = new QPushButton("নতুন বিভাগ (New Section)");
        add_section_button_->setStyleSheet("QPushButton { background-color: #44bd32; } QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");

        inject_heading_button_ = new QPushButton("ইনজেক্ট করুন (Inject)");
        inject_heading_button_->setStyleSheet("QPushButton { background-color: #8c7ae6; } QPushButton:hover { background-color: #9c88ff; }");
        inject_heading_button_->setEnabled(false);

        mode_label_ = new QLabel("মোড:");
        mode_dropdown_ = new QComboBox();
        mode_dropdown_->addItem("কপি মোড (Ctrl+C)");
        mode_dropdown_->addItem("সিলেক্ট মোড");

        // --- Layout Panels ---
        // 1. Subject Management Panel (Card)
        QFrame *subject_card = new QFrame();
        subject_card->setObjectName("card");
        QVBoxLayout *subject_layout = new QVBoxLayout(subject_card);
        subject_layout->setContentsMargins(12, 12, 12, 12);
        subject_layout->setSpacing(8);
        QLabel *subject_title = new QLabel("নোট বিষয় নির্বাচন (Subject Selection):");
        subject_title->setStyleSheet("font-weight: bold; color: #192a56;");
        subject_layout->addWidget(subject_title);
        
        QHBoxLayout *file_layout = new QHBoxLayout();
        file_layout->addWidget(subject_dropdown_, 1);
        file_layout->addWidget(add_subject_button_);
        file_layout->addWidget(open_file_button_);
        subject_layout->addLayout(file_layout);

        // 2. Capture Configuration Panel (Card)
        QFrame *capture_card = new QFrame();
        capture_card->setObjectName("card");
        QVBoxLayout *capture_layout = new QVBoxLayout(capture_card);
        capture_layout->setContentsMargins(12, 12, 12, 12);
        capture_layout->setSpacing(8);
        QLabel *capture_title = new QLabel("ক্যাপচার ও ইনপুট কনফিগারেশন (Capture Configuration):");
        capture_title->setStyleSheet("font-weight: bold; color: #192a56;");
        capture_layout->addWidget(capture_title);

        QHBoxLayout *options_layout = new QHBoxLayout();
        options_layout->addWidget(new QLabel("ফরম্যাট:"));
        options_layout->addWidget(format_dropdown_, 1);
        options_layout->addWidget(mode_label_);
        options_layout->addWidget(mode_dropdown_, 1);
        capture_layout->addLayout(options_layout);

        QHBoxLayout *section_layout = new QHBoxLayout();
        section_layout->addWidget(section_label_);
        section_layout->addWidget(section_dropdown_, 1);
        section_layout->addWidget(add_section_button_);
        capture_layout->addLayout(section_layout);

        QHBoxLayout *image_layout = new QHBoxLayout();
        image_layout->addWidget(add_image_button_, 1);
        capture_layout->addLayout(image_layout);

        // 3. Heading Management Panel (Card)
        QFrame *heading_card = new QFrame();
        heading_card->setObjectName("card");
        QVBoxLayout *heading_card_layout = new QVBoxLayout(heading_card);
        heading_card_layout->setContentsMargins(12, 12, 12, 12);
        heading_card_layout->setSpacing(8);
        QLabel *heading_title = new QLabel("টার্গেট শিরোনাম নিয়ন্ত্রণ (Target Heading Control):");
        heading_title->setStyleSheet("font-weight: bold; color: #192a56;");
        heading_card_layout->addWidget(heading_title);

        QHBoxLayout *heading_layout = new QHBoxLayout();
        heading_layout->addWidget(heading_label_);
        heading_layout->addWidget(select_heading_button_, 1);
        heading_card_layout->addLayout(heading_layout);

        QHBoxLayout *heading_actions_layout = new QHBoxLayout();
        heading_actions_layout->addWidget(append_to_heading_button_);
        heading_actions_layout->addWidget(inject_heading_button_);
        heading_actions_layout->addWidget(shift_heading_button_);
        heading_actions_layout->addWidget(delete_heading_button_);
        heading_card_layout->addLayout(heading_actions_layout);

        // Main Layout
        QVBoxLayout *main_layout = new QVBoxLayout(this);
        main_layout->setSpacing(10);
        main_layout->setContentsMargins(12, 12, 12, 12);

        main_layout->addWidget(status_label_);
        main_layout->addWidget(last_captured_label_);
        main_layout->addWidget(subject_card);
        main_layout->addWidget(capture_card);
        main_layout->addWidget(heading_card);

        settings_button_ = new QPushButton("সেটিংস (Settings)");
        settings_button_->setStyleSheet("QPushButton { background-color: #718093; } QPushButton:hover { background-color: #636e72; }");

        QHBoxLayout *control_layout1 = new QHBoxLayout();
        control_layout1->addWidget(start_button_);
        control_layout1->addWidget(stop_button_);
        control_layout1->addWidget(settings_button_);
        main_layout->addLayout(control_layout1);

        // --- Clipboard Timer ---
        clipboard_timer_ = new QTimer(this);
        clipboard_timer_->setInterval(1000); // Check every 1 second

        // --- Connections (Signals and Slots) ---
        connect(start_button_, &QPushButton::clicked, this, [this]() { this->start_monitoring(); });
        connect(stop_button_, &QPushButton::clicked, this, [this]() { this->stop_monitoring(); });
        connect(add_image_button_, &QPushButton::clicked, this, [this]() { this->add_clipboard_image(); });
        connect(add_subject_button_, &QPushButton::clicked, this, [this]() { this->add_subject(); });
        connect(open_file_button_, &QPushButton::clicked, this, [this]() { this->open_selected_file(); });
        connect(subject_dropdown_, &QComboBox::currentTextChanged, this, [this](const QString &text) { this->on_subject_changed(text); });
        connect(select_heading_button_, &QPushButton::clicked, this, [this]() { this->open_heading_select_dialog(); });
        connect(append_to_heading_button_, &QPushButton::clicked, this, [this]() { this->manual_append_to_heading(); });
        connect(delete_heading_button_, &QPushButton::clicked, this, [this]() { this->delete_selected_heading_section(); });
        connect(clipboard_timer_, &QTimer::timeout, this, [this]() { this->check_clipboard(); });
        connect(inject_heading_button_, &QPushButton::clicked, this, [this]() { this->inject_heading_from_clipboard(); });
        
        // Connect shift and add section
        connect(shift_heading_button_, &QPushButton::clicked, this, [this]() { this->shift_selected_heading_section(); });
        connect(add_section_button_, &QPushButton::clicked, this, [this]() { this->add_section(); });
        connect(settings_button_, &QPushButton::clicked, this, [this]() { this->open_settings_dialog(); });

        // --- Initial Population ---
        populate_subjects_from_disk();
        subject_dropdown_->setCurrentIndex(-1); // No initial selection
        update_status_label();
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        QWidget::closeEvent(event);
    }

private slots:
    void start_monitoring() {
        if (subject_dropdown_->currentIndex() == -1) {
            status_label_->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
            return;
        }
        is_running_ = true;
        
        restore_state_from_file(get_current_target_file());
        update_toc_in_file(get_current_target_file());
        
        QClipboard::Mode mode = (mode_dropdown_->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
        last_simplified_text_ = QGuiApplication::clipboard()->text(mode).simplified();
        
        clipboard_timer_->start();
        start_button_->setEnabled(false);
        stop_button_->setEnabled(true);
        subject_dropdown_->setEnabled(false);
        add_subject_button_->setEnabled(false);
        mode_dropdown_->setEnabled(false);
        update_status_label();
    }

    void stop_monitoring() {
        is_running_ = false;
        clipboard_timer_->stop();
        update_toc_in_file(get_current_target_file());
        start_button_->setEnabled(true);
        stop_button_->setEnabled(false);
        subject_dropdown_->setEnabled(true);
        add_subject_button_->setEnabled(true);
        mode_dropdown_->setEnabled(true);
        update_status_label();
    }

    void add_subject() {
        bool ok;
        QString text = QInputDialog::getText(this, "বিষয় যোগ করুন",
                                             "নতুন বিষয়ের নাম:", QLineEdit::Normal,
                                             "", &ok);
        if (ok && !text.isEmpty()) {
            // Avoid adding duplicates
            if (subject_dropdown_->findText(text) == -1) {
                subject_dropdown_->addItem(text);
                
                // --- Scalability Fix: Create file immediately ---
                QString filename = notes_dir_path_ + QDir::separator() + text + ".md";
                if (!QFile::exists(filename)) {
                    QFile file(filename);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.close();
                    }
                }
            }
            subject_dropdown_->setCurrentText(text);
        }
    }

    void check_clipboard() {
        QClipboard *clipboard = QGuiApplication::clipboard();
        QClipboard::Mode mode = (mode_dropdown_->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
        QString current_text = clipboard->text(mode);
        QString simplified_text = current_text.simplified();

        if (!simplified_text.isEmpty() && simplified_text != last_simplified_text_) {
            last_simplified_text_ = simplified_text;
            last_captured_label_->setText("শেষ ক্যাপচার: " + simplified_text);
            QString current_section = section_dropdown_->currentData().toString();
            write_to_file(simplified_text, current_section);
        }
    }

    void on_subject_changed(const QString &text) {
        update_status_label();
        open_file_button_->setEnabled(!text.isEmpty());
        inject_heading_button_->setEnabled(!text.isEmpty());

        if (!text.isEmpty()) {
            QString target_file = get_current_target_file();
            normalize_markdown_file(target_file);
            update_toc_in_file(target_file);
            restore_state_from_file(target_file);
            populate_headings_from_file();
        } else {
            all_headings_.clear();
            selected_heading_slug_ = "";
            selected_heading_title_ = "(শেষে নতুন করে যোগ করুন / Append to End)";
            if (select_heading_button_) {
                select_heading_button_->setText(selected_heading_title_);
                select_heading_button_->setEnabled(false);
            }
            delete_heading_button_->setEnabled(false);
            append_to_heading_button_->setEnabled(false);
            shift_heading_button_->setEnabled(false);
        }
    }

    void inject_heading_from_clipboard() {
        if (subject_dropdown_->currentIndex() == -1) {
            status_label_->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
            return;
        }

        QClipboard *clipboard = QGuiApplication::clipboard();
        QClipboard::Mode mode = (mode_dropdown_->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
        QString current_text = clipboard->text(mode);
        QString simplified_text = current_text.simplified();

        if (simplified_text.isEmpty()) {
            last_captured_label_->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো লেখা পাওয়া যায়নি।");
            return;
        }

        last_simplified_text_ = simplified_text;
        last_captured_label_->setText("শেষ ক্যাপচার (শিরোনাম হিসেবে ইনজেক্ট করা হয়েছে): " + simplified_text);
        
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়") return;

        restore_state_from_file(target_file);

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QDateTime now = QDateTime::currentDateTime();
            QString current_date = now.toString("dd MMMM, yyyy");
            
            if (current_date != last_date_) {
                outfile << "\n### ***" << current_date.toStdString() << "***\n";
                last_date_ = current_date;
            }
            
            QString title = simplified_text.trimmed();
            std::string slug = generate_slug(title);
            QString section = section_dropdown_->currentData().toString();
            
            outfile << "\n<h2 id=\"" << slug << "\" data-section=\"" << section.toStdString() << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">" 
                    << title.toStdString() << "</h2>\n";
            
            outfile.close();
            update_toc_in_file(target_file);
        } else {
            std::cerr << "Error: Could not open file " << target_file.toStdString() << std::endl;
            last_captured_label_->setText("ত্রুটি: ফাইলে লেখা যায়নি!");
        }
    }

    void open_selected_file() {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়" || !QFile::exists(target_file)) {
            status_label_->setText("অবস্থা: ফাইলটি খুঁজে পাওয়া যায়নি!");
            return;
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(target_file));
    }

    void add_clipboard_image() {
        if (subject_dropdown_->currentIndex() == -1) {
            status_label_->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
            return;
        }

        QClipboard *clipboard = QGuiApplication::clipboard();
        QImage image = clipboard->image();

        if (image.isNull()) {
            last_captured_label_->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো ছবি পাওয়া যায়নি।");
            return;
        }

        // Ensure images directory exists
        QString images_dir_path = notes_dir_path_ + QDir::separator() + "images";
        QDir img_dir(images_dir_path);
        if (!img_dir.exists()) {
            img_dir.mkpath(".");
        }

        QDateTime now = QDateTime::currentDateTime();
        QString timestamp = now.toString("yyyyMMdd_hhmmss_zzz");
        QString filename = QString("img_%1.png").arg(timestamp);
        QString filepath = images_dir_path + QDir::separator() + filename;

        if (image.save(filepath, "PNG")) {
            write_image_to_file(filename);
            last_captured_label_->setText("শেষ ক্যাপচার: ছবি যোগ করা হয়েছে (" + filename + ")");
        } else {
            last_captured_label_->setText("ত্রুটি: ছবি সংরক্ষণ করা যায়নি!");
        }
    }

    void update_button_states() {
        bool has_subject = (subject_dropdown_->currentIndex() != -1);
        bool has_selected_heading = (!selected_heading_slug_.isEmpty());
        
        delete_heading_button_->setEnabled(has_subject && has_selected_heading);
        append_to_heading_button_->setEnabled(has_subject && has_selected_heading);
        shift_heading_button_->setEnabled(has_subject && has_selected_heading);
    }

    void open_heading_select_dialog() {
        HeadingSelectDialog dlg(all_headings_, selected_heading_slug_, this);
        if (dlg.exec() == QDialog::Accepted) {
            selected_heading_slug_ = dlg.get_selected_slug();
            selected_heading_title_ = dlg.get_selected_title();
            select_heading_button_->setText(selected_heading_title_);
            update_button_states();
        }
    }

    void add_section() {
        bool ok;
        QString text = QInputDialog::getText(this, "বিভাগ যোগ করুন",
                                             "নতুন বিভাগের নাম:", QLineEdit::Normal,
                                             "", &ok);
        if (ok && !text.isEmpty()) {
            QString slug = QString::fromStdString(generate_slug(text));
            // Check if already exists in section_dropdown_
            bool exists = false;
            for (int i = 0; i < section_dropdown_->count(); ++i) {
                if (section_dropdown_->itemData(i).toString() == slug) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                section_dropdown_->addItem(QString("%1 (%2)").arg(text, slug.toUpper()), slug);
            }
            section_dropdown_->setCurrentIndex(section_dropdown_->count() - 1);
        }
    }

    void shift_selected_heading_section() {
        debugLog(QString("shift_selected_heading_section called"));
        if (subject_dropdown_->currentIndex() == -1) {
            debugLog("shift_selected_heading_section: no subject selected");
            return;
        }
        if (selected_heading_slug_.isEmpty()) {
            debugLog("shift_selected_heading_section: no selected heading to shift");
            return;
        }

        QString source_slug = selected_heading_slug_;
        QString target_file = get_current_target_file();
        debugLog(QString("shift_selected_heading_section: source_slug='%1', target_file='%2'")
                 .arg(source_slug, target_file));

        // 1. Gather all potential targets (all other headings in the current file)
        QStringList target_titles;
        QStringList target_slugs;
        
        target_titles.append("(শেষে স্থানান্তর করুন / Move to End)");
        target_slugs.append("");

        for (const NoteItem &item : all_headings_) {
            if (item.slug != source_slug) {
                if (item.type == "heading") {
                    target_titles.append(QString("%1 (id: %2)").arg(item.title, item.slug));
                } else {
                    target_titles.append(QString("  ↳ %1 (id: %2)").arg(item.title, item.slug));
                }
                target_slugs.append(item.slug);
            }
        }

        // 2. Show a dialog to select the target heading
        bool ok;
        QString target_selection = QInputDialog::getItem(this, "সেকশন স্থানান্তর", 
                                                         "কোথায় স্থানান্তর করতে চান তা নির্বাচন করুন:", 
                                                         target_titles, 0, false, &ok);
        if (!ok) {
            debugLog("shift_selected_heading_section: user cancelled target dialog");
            return;
        }

        int selected_idx = target_titles.indexOf(target_selection);
        if (selected_idx == -1) {
            debugLog("shift_selected_heading_section: target selection not found in list");
            return;
        }
        QString target_slug = target_slugs.at(selected_idx);
        debugLog(QString("shift_selected_heading_section: selected target_slug='%1'").arg(target_slug));

        // 3. Read content
        QFile file(target_file);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            last_captured_label_->setText("ত্রুটি: ফাইলটি খোলা যায়নি!");
            debugLog("shift_selected_heading_section: failed to open target file for reading");
            return;
        }
        
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // 4. Find and extract source section bounds
        int src_start = -1;
        int src_end = -1;
        bool is_html = false;
        bool found = false;

        if (get_heading_bounds(content, source_slug, src_start, src_end, is_html)) {
            found = true;
            debugLog(QString("shift_selected_heading_section: source found in headings (H2), bounds=%1-%2, is_html=%3")
                     .arg(QString::number(src_start), QString::number(src_end), QString(is_html ? "true" : "false")));
        } else if (get_subheading_bounds(content, source_slug, src_start, src_end)) {
            found = true;
            debugLog(QString("shift_selected_heading_section: source found in subheadings (H3), bounds=%1-%2")
                     .arg(QString::number(src_start), QString::number(src_end)));
        }

        if (!found) {
            last_captured_label_->setText("ত্রুটি: স্থানান্তর করার জন্য উৎস সেকশন পাওয়া যায়নি!");
            debugLog("shift_selected_heading_section: source section bounds not found!");
            return;
        }

        // Extract chunk and remove from original content
        QString source_chunk = content.mid(src_start, src_end - src_start);
        content.remove(src_start, src_end - src_start);

        // 5. Find insert position in the modified content
        int insert_pos = -1;
        if (target_slug.isEmpty()) {
            // Move to End
            insert_pos = content.length();
            debugLog(QString("shift_selected_heading_section: moving to end, insert_pos=%1").arg(insert_pos));
        } else {
            // Find target section bounds in modified content
            int tgt_start = -1;
            int tgt_end = -1;
            bool tgt_is_html = false;
            
            // Check if it's heading (H2) or subheading (H3)
            bool tgt_found_h2 = get_heading_bounds(content, target_slug, tgt_start, tgt_end, tgt_is_html);
            if (tgt_found_h2) {
                insert_pos = tgt_end;
                debugLog(QString("shift_selected_heading_section: target is H2, insert_pos=%1").arg(insert_pos));
            } else {
                bool tgt_found_h3 = get_subheading_insert_pos(content, target_slug, insert_pos);
                if (!tgt_found_h3) {
                    last_captured_label_->setText("ত্রুটি: গন্তব্য সেকশন খুঁজে পাওয়া যায়নি!");
                    debugLog("shift_selected_heading_section: target subheading not found in modified content");
                    return;
                }
                debugLog(QString("shift_selected_heading_section: target is H3, insert_pos=%1").arg(insert_pos));
            }
        }

        // Insert the chunk at target position
        // Make sure there are newlines around it
        if (insert_pos > 0 && content[insert_pos - 1] != '\n') {
            source_chunk.prepend("\n");
        }
        content.insert(insert_pos, source_chunk);

        // 6. Write final content back
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << content;
            file.close();
            
            last_captured_label_->setText(QString("স্থানান্তর সফল হয়েছে: %1").arg(source_slug));
            debugLog("shift_selected_heading_section: shift complete and file written back");
            
            // Refresh headings dropdown & TOC
            update_toc_in_file(target_file);
            populate_headings_from_file();
        } else {
            last_captured_label_->setText("ত্রুটি: ফাইলে স্থানান্তর সম্পন্ন করা যায়নি!");
            debugLog("shift_selected_heading_section: failed to open target file for writing");
        }
    }

    void manual_append_to_heading() {
        if (subject_dropdown_->currentIndex() == -1) return;
        if (selected_heading_slug_.isEmpty()) return;
        
        QClipboard *clipboard = QGuiApplication::clipboard();
        QClipboard::Mode mode = (mode_dropdown_->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
        QString current_text = clipboard->text(mode);
        QString simplified_text = current_text.simplified();

        if (simplified_text.isEmpty()) {
            last_captured_label_->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো লেখা পাওয়া যায়নি।");
            return;
        }

        QString target_file = get_current_target_file();
        QString slug = selected_heading_slug_;
        
        if (append_content_to_heading(target_file, slug, simplified_text)) {
            update_toc_in_file(target_file);
            last_captured_label_->setText("শেষ ক্যাপচার (নির্বাচিত শিরোনামে ম্যানুয়ালি যুক্ত করা হয়েছে): " + simplified_text);
        } else {
            last_captured_label_->setText("ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!");
        }
    }

    void delete_selected_heading_section() {
        if (subject_dropdown_->currentIndex() == -1) return;
        if (selected_heading_slug_.isEmpty()) return;
        
        QString slug = selected_heading_slug_;
        QString target_file = get_current_target_file();
        
        QFile file(target_file);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            last_captured_label_->setText("ত্রুটি: ফাইলটি খোলা যায়নি!");
            return;
        }
        
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        
        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        bool found = false;
        
        if (get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
            found = true;
        } else if (get_subheading_bounds(content, slug, start_pos, end_pos)) {
            found = true;
        }
        
        if (found) {
            QString deleted_chunk = content.mid(start_pos, end_pos - start_pos);
            
            // Remove the chunk from content
            content.remove(start_pos, end_pos - start_pos);
            
            // Write the remaining content back to file
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                QTextStream out(&file);
                out << content;
                file.close();
            }
            
            // Save the deleted chunk to another folder in a txt file
            QString del_filename = QString("%1_%2_%3.txt")
                                   .arg(subject_dropdown_->currentText())
                                   .arg(slug)
                                   .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            
            // Ensure deleted directory exists
            QString deleted_dir_path = notes_dir_path_ + QDir::separator() + "deleted";
            QDir del_dir(deleted_dir_path);
            if (!del_dir.exists()) {
                del_dir.mkpath(".");
            }
            
            QString del_filepath = deleted_dir_path + QDir::separator() + del_filename;
            QFile del_file(del_filepath);
            if (del_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream del_out(&del_file);
                del_out << deleted_chunk;
                del_file.close();
                last_captured_label_->setText(QString("মুছে ফেলা হয়েছে এবং ব্যাকআপ রাখা হয়েছে: %1").arg(del_filename));
            } else {
                last_captured_label_->setText("মুছে ফেলা হয়েছে কিন্তু ব্যাকআপ রাখা যায়নি!");
            }
            
            // Refresh headings dropdown & TOC
            update_toc_in_file(target_file);
            selected_heading_slug_ = "";
            selected_heading_title_ = "(শেষে নতুন করে যোগ করুন / Append to End)";
            populate_headings_from_file();
        } else {
            last_captured_label_->setText("ত্রুটি: শিরোনাম বা উপ-শিরোনামটি খুঁজে পাওয়া যায়নি!");
        }
    }

private:
    void populate_subjects_from_disk() {
        // Populate dropdown from all .md files in the directory
        QDir directory(notes_dir_path_);
        QStringList md_files = directory.entryList(QStringList() << "*.md", QDir::Files);
        
        subject_dropdown_->clear();
        for (QString filename : md_files) {
            QString full_path = notes_dir_path_ + QDir::separator() + filename;
            normalize_markdown_file(full_path);
            update_toc_in_file(full_path);
            
            filename.chop(3); // Remove ".md"
            subject_dropdown_->addItem(filename);
        }
    }

    QString detect_section_from_title(const QString &title) {
        QString lower_title = title.toLower().trimmed();
        
        struct MapEntry {
            QString keyword;
            QString section;
        };
        QList<MapEntry> mappings = {
            {"environment", "environment"}, {"পরিবেশ", "environment"},
            {"energy", "energy"}, {"জ্বালানি", "energy"},
            {"economy", "economy"}, {"অর্থনীতি", "economy"},
            {"culture", "culture"}, {"সংস্কৃতি", "culture"},
            {"geography", "geography"}, {"ভূগোল", "geography"},
            {"population", "population"}, {"জনসংখ্যা", "population"},
            {"law-constitution", "law-constitution"}, {"আইন ও সংবিধান", "law-constitution"},
            {"politics", "politics"}, {"রাজনীতি", "politics"},
            {"freedom-fight", "freedom-fight"}, {"মুক্তিযুদ্ধ", "freedom-fight"},
            {"agriculture", "agriculture"}, {"কৃষি", "agriculture"},
            {"history", "history"}, {"ইতিহাস", "history"},
            {"education", "education"}, {"শিক্ষা", "education"},
            {"health", "health"}, {"স্বাস্থ্য", "health"},
            {"science-tech", "science-tech"}, {"বিজ্ঞান ও প্রযুক্তি", "science-tech"}, {"বিজ্ঞান", "science-tech"}, {"প্রযুক্তি", "science-tech"},
            {"foreign-policy", "foreign-policy"}, {"পররাষ্ট্রনীতি", "foreign-policy"},
            {"administration", "administration"}, {"প্রশাসন", "administration"}
        };
        
        for (const auto &entry : mappings) {
            if (lower_title.contains(entry.keyword)) {
                return entry.section;
            }
        }
        return "others";
    }

    void normalize_markdown_file(const QString &file_path) {
        QFile file(file_path);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // 1. Remove existing TOC block if present
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

        QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression md_regex("^(#{1,3})\\s+(?!\\*\\*\\*)(.*?)$");
        QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");
        QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QString trimmed_line = line.trimmed();

            QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
            QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
            QRegularExpressionMatch md_match = md_regex.match(trimmed_line);

            if (trimmed_line.contains("<div") && (trimmed_line.contains("border") || trimmed_line.contains("background-color"))) {
                // Skip the container box opening
                continue;
            } else if (trimmed_line == "</div>") {
                // Skip the container box closing
                continue;
            } else if (date_match.hasMatch()) {
                output_lines.append(line);
            } else if (h2_match.hasMatch()) {
                QString attributes = h2_match.captured(1);
                QString title = h2_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;"; // default
                QRegularExpressionMatch style_match = style_regex.match(attributes);
                if (style_match.hasMatch()) {
                    style = style_match.captured(1);
                }

                QString section = detect_section_from_title(title);
                QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                }

                QString rewritten_line = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                         .arg(slug, section, style, title);
                output_lines.append(rewritten_line);
            } else if (md_match.hasMatch()) {
                int level = md_match.captured(1).length();
                if (level == 3 && trimmed_line.contains("***")) {
                    output_lines.append(line);
                    continue;
                }

                QString rest = md_match.captured(2).trimmed();
                QString section = detect_section_from_title(rest);
                QRegularExpressionMatch section_match = md_section_regex.match(rest);
                QString title = rest;
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                    title = rest.left(section_match.capturedStart()).trimmed();
                }

                QString slug = QString::fromStdString(generate_slug(title));

                if (level == 2) {
                    QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;";
                    QString html_heading = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                           .arg(slug, section, style, title);
                    output_lines.append(html_heading);
                } else if (level == 3) {
                    QString style = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
                    QString html_subheading = QString("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                                              .arg(slug, style, title);
                    output_lines.append(html_subheading);
                }
            } else {
                output_lines.append(line);
            }
        }

        QString final_content = output_lines.join('\n');

        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << final_content << "\n";
            file.close();
        }
    }

    QString get_current_target_file() {
        if (subject_dropdown_->currentIndex() == -1) {
            return "নির্বাচিত নয়";
        }
        return notes_dir_path_ + QDir::separator() + subject_dropdown_->currentText() + ".md";
    }

    void update_status_label() {
        QString status_text = is_running_ ? "চলছে" : "বন্ধ";
        QString target_file = get_current_target_file();
        if (target_file != "নির্বাচিত নয়") {
            QFileInfo fileInfo(target_file);
            target_file = fileInfo.fileName();
        }
        status_label_->setText(QString("অবস্থা: %1 | লক্ষ্য: %2").arg(status_text, target_file));
    }



    void write_image_to_file(const QString &image_filename) {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়") return;

        restore_state_from_file(target_file);

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QDateTime now = QDateTime::currentDateTime();
            QString current_date = now.toString("dd MMMM, yyyy");
            
            if (current_date != last_date_) {
                outfile << "\n### ***" << current_date.toStdString() << "***\n";
                last_date_ = current_date;
            }
            
            outfile << "\n![Image](images/" << image_filename.toStdString() << ")\n\n";
            outfile.close();
            update_toc_in_file(target_file);
        } else {
            std::cerr << "Error: Could not open file " << target_file.toStdString() << std::endl;
            last_captured_label_->setText("ত্রুটি: ছবি ফাইলে যোগ করা যায়নি!");
        }
    }

    void write_to_file(const QString &processed_text, const QString &section = "others") {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়") return;

        // Check if we have a selected heading
        if (!selected_heading_slug_.isEmpty()) {
            QString slug = selected_heading_slug_;
            if (append_content_to_heading(target_file, slug, processed_text)) {
                update_toc_in_file(target_file);
                last_captured_label_->setText("শেষ ক্যাপচার (নির্বাচিত শিরোনামে যুক্ত করা হয়েছে): " + processed_text);
                return;
            } else {
                last_captured_label_->setText("ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!");
            }
        }

        restore_state_from_file(target_file);

        std::ofstream outfile;
        outfile.open(target_file.toStdString(), std::ios_base::app);
        
        if (outfile.is_open()) {
            QDateTime now = QDateTime::currentDateTime();
            QString current_date = now.toString("dd MMMM, yyyy");
            
            if (current_date != last_date_) {
                outfile << "\n### ***" << current_date.toStdString() << "***\n";
                last_date_ = current_date;
            }
            
            int format_index = format_dropdown_->currentIndex();
            if (!processed_text.isEmpty()) {
                if (format_index == 1) { // Heading Mode
                    QString title = processed_text.trimmed();
                    std::string slug = generate_slug(title);
                    outfile << "\n<h2 id=\"" << slug << "\" data-section=\"" << section.toStdString() << "\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">" 
                            << title.toStdString() << "</h2>\n";
                } else if (format_index == 2) { // Subheading Mode
                    QString title = processed_text.trimmed();
                    std::string slug = generate_slug(title);
                    outfile << "\n<h3 id=\"" << slug << "\" style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">" 
                            << title.toStdString() << "</h3>\n";
                } else { // Point Mode
                    outfile << "- " << processed_text.trimmed().toStdString() << "\n";
                }
            }
            
            outfile.close();
            update_toc_in_file(target_file);
            if (format_index == 1) {
                populate_headings_from_file();
            }
        } else {
            std::cerr << "Error: Could not open file " << target_file.toStdString() << std::endl;
            last_captured_label_->setText("ত্রুটি: ফাইলে লেখা যায়নি!");
        }
    }

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

    void restore_state_from_file(const QString &file_path) {
        last_date_ = "";
        
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
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
        
        last_date_ = last_found_date;
    }

    void update_toc_in_file(const QString &file_path) {
        QFile file(file_path);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // 1. Remove all existing TOC blocks recursively if present
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

        // 2. Parse lines to find headings and dates
        QStringList lines = clean_content.split('\n');
        
        struct HeadingInfo {
            int index;
            QString title;
            QString slug;
            QString date;
            bool is_html;
            QString style;
            int level;
            QString section;
        };
        
        QList<HeadingInfo> headings;
        QString current_date = "";
        
        QRegularExpression date_regex("^###\\s*(?:\\*\\*\\*)?\\s*([0-9০-৯]{1,2}\\s+(?:January|February|March|April|May|June|July|August|September|October|November|December|জানুয়ারি|ফেব্রুয়ারি|মার্চ|এপ্রিল|মে|জুন|জুলাই|আগস্ট|সেপ্টেম্বর|অক্টোবর|নভেম্বর|ডিসেম্বর)[,\\s]+[0-9০-৯]{4})\\s*(?:\\*\\*\\*)?$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression h2_regex("<h2([^>]*)>(.*?)</h2>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression h3_regex("<h3([^>]*)>(.*?)</h3>", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression style_regex("style=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression md_regex("^(#{2})\\s+(.*?)$");
        QRegularExpression md_sub_regex("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$");
        QRegularExpression md_section_regex("<!--\\s*section:([\\w-]+)\\s*-->");

        QStringList processed_lines;
        int heading_counter = 0;

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QString trimmed_line = line.trimmed();

            QRegularExpressionMatch date_match = date_regex.match(trimmed_line);
            QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
            QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
            QRegularExpressionMatch md_match = md_regex.match(trimmed_line);
            QRegularExpressionMatch md_sub_match = md_sub_regex.match(trimmed_line);

            if (date_match.hasMatch()) {
                current_date = date_match.captured(1).trimmed();
                processed_lines.append(line);
            } else if (h2_match.hasMatch()) {
                heading_counter++;
                QString attributes = h2_match.captured(1);
                QString title = h2_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                // Extract existing style
                QString style = "color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;"; // default
                QRegularExpressionMatch style_match = style_regex.match(attributes);
                if (style_match.hasMatch()) {
                    style = style_match.captured(1);
                }

                // Extract existing data-section
                QString section = "others";
                QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                }

                if (section.trimmed().isEmpty()) {
                    section = "others";
                }

                HeadingInfo info;
                info.index = heading_counter;
                info.title = title;
                info.slug = slug;
                info.date = current_date;
                info.is_html = true;
                info.style = style;
                info.level = 2;
                info.section = section;
                headings.append(info);

                // Rewrite the line to guarantee a valid id matching the slug and preserve attributes
                QString rewritten_line = QString("<h2 id=\"%1\" data-section=\"%2\" style=\"%3\">%4</h2>")
                                         .arg(slug, section, style, title);
                processed_lines.append(rewritten_line);
            } else if (h3_match.hasMatch()) {
                QString attributes = h3_match.captured(1);
                QString title = h3_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));
                QString style = "color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;";
                QRegularExpressionMatch style_match = style_regex.match(attributes);
                if (style_match.hasMatch()) {
                    style = style_match.captured(1);
                }
                
                QString rewritten_line = QString("<h3 id=\"%1\" style=\"%2\">%3</h3>")
                                         .arg(slug, style, title);
                processed_lines.append(rewritten_line);
            } else if (md_match.hasMatch()) {
                heading_counter++;
                QString rest = md_match.captured(2).trimmed();

                QString section = "others";
                QRegularExpressionMatch section_match = md_section_regex.match(rest);
                QString title = rest;
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                    title = rest.left(section_match.capturedStart()).trimmed();
                }

                if (section.trimmed().isEmpty()) {
                    section = "others";
                }

                QString slug = QString::fromStdString(generate_slug(title));

                HeadingInfo info;
                info.index = heading_counter;
                info.title = title;
                info.slug = slug;
                info.date = current_date;
                info.is_html = false;
                info.style = "";
                info.level = 2;
                info.section = section;
                headings.append(info);

                processed_lines.append(line);
            } else if (md_sub_match.hasMatch()) {
                processed_lines.append(line);
            } else {
                processed_lines.append(line);
            }
        }

        // 3. Generate the TOC content
        QString toc_block = "";
        if (!headings.isEmpty()) {
            toc_block += "<!-- TOC_START -->\n";
            toc_block += "## সূচিপত্র (Table of Contents)\n\n";

            struct SectionDef {
                QString key;
                QString bangla;
                QString english;
            };

            QList<SectionDef> sections_list = {
                {"environment", "পরিবেশ", "Environment"},
                {"energy", "জ্বালানি", "Energy"},
                {"economy", "অর্থনীতি", "Economy"},
                {"culture", "সংস্কৃতি", "Culture"},
                {"geography", "ভূগোল", "Geography"},
                {"population", "জনসংখ্যা", "Population"},
                {"law-constitution", "আইন ও সংবিধান", "Law-Constitution"},
                {"politics", "রাজনীতি", "Politics"},
                {"freedom-fight", "মুক্তিযুদ্ধ", "Freedom-Fight"},
                {"agriculture", "কৃষি", "Agriculture"},
                {"history", "ইতিহাস", "History"},
                {"education", "শিক্ষা", "Education"},
                {"health", "স্বাস্থ্য", "Health"},
                {"science-tech", "বিজ্ঞান ও প্রযুক্তি", "Science-Tech"},
                {"foreign-policy", "পররাষ্ট্রনীতি", "Foreign-Policy"},
                {"administration", "প্রশাসন", "Administration"}
            };

            // Dynamically collect custom sections
            for (const HeadingInfo &info : headings) {
                if (info.section.isEmpty() || info.section == "others") continue;
                bool exists = false;
                for (const SectionDef &sec : sections_list) {
                    if (sec.key == info.section) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    // Try to look it up in the dropdown to get display names
                    QString bangla_name = info.section;
                    QString english_name = info.section;
                    for (int d_idx = 0; d_idx < section_dropdown_->count(); ++d_idx) {
                        if (section_dropdown_->itemData(d_idx).toString() == info.section) {
                            QString full_text = section_dropdown_->itemText(d_idx);
                            int paren_idx = full_text.indexOf('(');
                            if (paren_idx != -1) {
                                bangla_name = full_text.left(paren_idx).trimmed();
                                QString eng = full_text.mid(paren_idx + 1);
                                if (eng.endsWith(')')) eng.chop(1);
                                english_name = eng.trimmed();
                            } else {
                                bangla_name = full_text;
                                english_name = full_text;
                            }
                            break;
                        }
                    }
                    sections_list.append({info.section, bangla_name, english_name});
                }
            }
            
            // Add "others" last
            sections_list.append({"others", "অন্যান্য", "Others"});

            for (const SectionDef &sec : sections_list) {
                QList<HeadingInfo> sec_headings;
                for (const HeadingInfo &info : headings) {
                    if (info.section == sec.key) {
                        sec_headings.append(info);
                    }
                }

                if (!sec_headings.isEmpty()) {
                    toc_block += QString("### %1 (%2)\n").arg(sec.bangla, sec.english);
                    toc_block += "| পৃষ্ঠা (Page) | তারিখ (Date) | আইডি (ID) | শিরোনাম (Chapter/Topic) |\n";
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
        }

        // 4. Combine TOC and processed lines
        QString final_content = "";
        QString body_content = processed_lines.join('\n').trimmed();
        
        if (!toc_block.isEmpty()) {
            final_content = toc_block + "\n\n" + body_content;
        } else {
            final_content = body_content;
        }
        
        // 5. Write back to file
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << final_content << "\n";
            file.close();
        }
    }



    void parse_note_structure(const QString &file_path, QList<NoteItem> &items) {
        items.clear();
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

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
        QRegularExpression md_regex("^(#{2})\\s+(.*?)$"); // ## heading
        QRegularExpression md_sub_regex("^(#{3})\\s+(?!\\*\\*\\*)(.*?)$"); // ### subheading (excluding date)
        QRegularExpression section_attr_regex("data-section=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);

        QString current_h2_slug = "";

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            QString trimmed_line = line.trimmed();

            QRegularExpressionMatch h2_match = h2_regex.match(trimmed_line);
            QRegularExpressionMatch h3_match = h3_regex.match(trimmed_line);
            QRegularExpressionMatch md_match = md_regex.match(trimmed_line);
            QRegularExpressionMatch md_sub_match = md_sub_regex.match(trimmed_line);

            if (h2_match.hasMatch()) {
                QString attributes = h2_match.captured(1);
                QString title = h2_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));
                
                QString section = "others";
                QRegularExpressionMatch section_match = section_attr_regex.match(attributes);
                if (section_match.hasMatch()) {
                    section = section_match.captured(1);
                }

                if (!section.isEmpty() && section != "others") {
                    bool exists = false;
                    for (int idx = 0; idx < section_dropdown_->count(); ++idx) {
                        if (section_dropdown_->itemData(idx).toString() == section) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        QString display_name = section;
                        if (display_name.length() > 0) {
                            display_name[0] = display_name[0].toUpper();
                        }
                        section_dropdown_->addItem(QString("%1 (%2)").arg(display_name, section.toUpper()), section);
                    }
                }

                NoteItem item = {title, slug, "heading", section, ""};
                items.append(item);
                current_h2_slug = slug;
            } else if (md_match.hasMatch()) {
                QString title = md_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));
                QString section = detect_section_from_title(title);

                if (!section.isEmpty() && section != "others") {
                    bool exists = false;
                    for (int idx = 0; idx < section_dropdown_->count(); ++idx) {
                        if (section_dropdown_->itemData(idx).toString() == section) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        QString display_name = section;
                        if (display_name.length() > 0) {
                            display_name[0] = display_name[0].toUpper();
                        }
                        section_dropdown_->addItem(QString("%1 (%2)").arg(display_name, section.toUpper()), section);
                    }
                }

                NoteItem item = {title, slug, "heading", section, ""};
                items.append(item);
                current_h2_slug = slug;
            } else if (h3_match.hasMatch()) {
                QString title = h3_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                NoteItem item = {title, slug, "subheading", "others", current_h2_slug};
                items.append(item);
            } else if (md_sub_match.hasMatch()) {
                QString title = md_sub_match.captured(2).trimmed();
                QString slug = QString::fromStdString(generate_slug(title));

                NoteItem item = {title, slug, "subheading", "others", current_h2_slug};
                items.append(item);
            }
        }
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

    void populate_headings_from_file() {
        QString target_file = get_current_target_file();
        if (target_file == "নির্বাচিত নয়" || !QFile::exists(target_file)) {
            all_headings_.clear();
            selected_heading_slug_ = "";
            selected_heading_title_ = "(শেষে নতুন করে যোগ করুন / Append to End)";
            if (select_heading_button_) {
                select_heading_button_->setText(selected_heading_title_);
                select_heading_button_->setEnabled(false);
            }
            update_button_states();
            return;
        }

        all_headings_.clear();
        parse_note_structure(target_file, all_headings_);
        
        // Save outline tree file
        save_tree_file(target_file, all_headings_);

        if (select_heading_button_) {
            select_heading_button_->setEnabled(true);
        }

        // Verify if previously selected heading still exists
        bool exists = false;
        if (!selected_heading_slug_.isEmpty()) {
            for (const auto &item : all_headings_) {
                if (item.slug == selected_heading_slug_) {
                    exists = true;
                    selected_heading_title_ = item.title;
                    break;
                }
            }
        }
        if (!exists) {
            selected_heading_slug_ = "";
            selected_heading_title_ = "(শেষে নতুন করে যোগ করুন / Append to End)";
        }

        if (select_heading_button_) {
            select_heading_button_->setText(selected_heading_title_);
        }
        update_button_states();
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

    bool append_content_to_heading(const QString &file_path, const QString &slug, const QString &processed_text) {
        QFile file(file_path);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            return false;
        }
        
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();
        
        int start_pos = -1;
        int end_pos = -1;
        bool is_html = false;
        
        if (get_heading_bounds(content, slug, start_pos, end_pos, is_html)) {
            QString to_append = "";
            int format_index = format_dropdown_->currentIndex();
            if (format_index == 2) {
                QString sub_slug = QString::fromStdString(generate_slug(processed_text));
                to_append = QString("\n<h3 id=\"%1\" style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%2</h3>\n")
                            .arg(sub_slug, processed_text.trimmed());
            } else if (format_index == 1) {
                QString main_slug = QString::fromStdString(generate_slug(processed_text));
                QString section = section_dropdown_->currentData().toString();
                to_append = QString("\n<h2 id=\"%1\" data-section=\"%2\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">%3</h2>\n")
                            .arg(main_slug, section, processed_text.trimmed());
            } else {
                to_append = QString("- %1\n").arg(processed_text.trimmed());
            }
            
            if (end_pos > 0 && content[end_pos - 1] != '\n') {
                to_append.prepend("\n");
            }
            content.insert(end_pos, to_append);
            
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                QTextStream out(&file);
                out << content;
                file.close();
                return true;
            }
        } else {
            int insert_pos = -1;
            if (get_subheading_insert_pos(content, slug, insert_pos)) {
                QString to_append = "";
                int format_index = format_dropdown_->currentIndex();
                if (format_index == 2) {
                    QString sub_slug = QString::fromStdString(generate_slug(processed_text));
                    to_append = QString("\n<h3 id=\"%1\" style=\"color: #2980b9; font-weight: bold; font-style: italic; margin-top: 10px; margin-bottom: 5px;\">%2</h3>\n")
                                .arg(sub_slug, processed_text.trimmed());
                } else if (format_index == 1) {
                    QString main_slug = QString::fromStdString(generate_slug(processed_text));
                    QString section = section_dropdown_->currentData().toString();
                    to_append = QString("\n<h2 id=\"%1\" data-section=\"%2\" style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">%3</h2>\n")
                                .arg(main_slug, section, processed_text.trimmed());
                } else {
                    to_append = QString("- %1\n").arg(processed_text.trimmed());
                }
                
                if (insert_pos > 0 && content[insert_pos - 1] != '\n') {
                    to_append.prepend("\n");
                }
                content.insert(insert_pos, to_append);
                
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
                    QTextStream out(&file);
                    out << content;
                    file.close();
                    return true;
                }
            }
        }
        return false;
    }

    // --- State Variables ---
    bool is_running_;
    QString last_simplified_text_;
    QString last_date_;
    QString notes_dir_path_;

    // --- UI Pointers ---
    QLabel *status_label_;
    QLabel *last_captured_label_;
    QPushButton *start_button_;
    QPushButton *stop_button_;
    QPushButton *add_image_button_;
    QComboBox *subject_dropdown_;
    QPushButton *add_subject_button_;
    QPushButton *open_file_button_;
    QComboBox *format_dropdown_;
    QTimer *clipboard_timer_;
    QLabel *mode_label_;
    QComboBox *mode_dropdown_;
    QLabel *section_label_;
    QComboBox *section_dropdown_;
    QPushButton *inject_heading_button_;
    QLabel *heading_label_;
    QPushButton *select_heading_button_;
    QString selected_heading_slug_;
    QString selected_heading_title_;
    QPushButton *append_to_heading_button_;
    QPushButton *delete_heading_button_;
    QList<NoteItem> all_headings_;
    QPushButton *shift_heading_button_;
    QPushButton *add_section_button_;
    QPushButton *settings_button_;
    QList<ShortcutConfig> shortcut_configs_;

    void init_shortcut_configs() {
        shortcut_configs_ = {
            {"start", "শুরু করুন (Start)", "Start Monitoring", "Ctrl+Shift+S", QKeySequence("Ctrl+Shift+S"), nullptr},
            {"stop", "থামুন (Stop)", "Stop Monitoring", "Ctrl+Shift+T", QKeySequence("Ctrl+Shift+T"), nullptr},
            {"add_image", "ছবি যুক্ত করুন (Add Image)", "Add Clipboard Image", "Ctrl+Shift+I", QKeySequence("Ctrl+Shift+I"), nullptr},
            {"new_subject", "নতুন বিষয় (New Subject)", "Create New Subject", "Ctrl+Shift+N", QKeySequence("Ctrl+Shift+N"), nullptr},
            {"open_note", "নোট খুলুন (Open Note)", "Open Active Note", "Ctrl+Shift+O", QKeySequence("Ctrl+Shift+O"), nullptr},
            {"append", "যুক্ত করুন (Append)", "Append to Heading", "Ctrl+Shift+A", QKeySequence("Ctrl+Shift+A"), nullptr},
            {"inject", "ইনজেক্ট করুন (Inject)", "Inject Heading", "Ctrl+Shift+J", QKeySequence("Ctrl+Shift+J"), nullptr},
            {"shift", "স্থানান্তর (Shift)", "Shift Heading Section", "Ctrl+Shift+H", QKeySequence("Ctrl+Shift+H"), nullptr},
            {"delete", "মুছে ফেলুন (Delete)", "Delete Heading Section", "Ctrl+Shift+D", QKeySequence("Ctrl+Shift+D"), nullptr},
            {"new_section", "নতুন বিভাগ (New Section)", "Add Custom Section", "Ctrl+Shift+K", QKeySequence("Ctrl+Shift+K"), nullptr},
            {"toggle_format", "ফরম্যাট পরিবর্তন (Toggle Format)", "Toggle Capture Format", "Ctrl+Shift+F", QKeySequence("Ctrl+Shift+F"), nullptr},
            {"toggle_section", "বিভাগ পরিবর্তন (Toggle Section)", "Toggle Section Category", "Ctrl+Shift+G", QKeySequence("Ctrl+Shift+G"), nullptr}
        };
    }

    void load_settings() {
        QSettings settings(notes_dir_path_ + QDir::separator() + "settings.ini", QSettings::IniFormat);
        settings.beginGroup("Shortcuts");
        for (auto &cfg : shortcut_configs_) {
            QString key_str = settings.value(cfg.action_id, cfg.default_key).toString();
            cfg.current_key = QKeySequence(key_str);
        }
        settings.endGroup();
    }

    void save_settings() {
        QSettings settings(notes_dir_path_ + QDir::separator() + "settings.ini", QSettings::IniFormat);
        settings.beginGroup("Shortcuts");
        for (const auto &cfg : shortcut_configs_) {
            settings.setValue(cfg.action_id, cfg.current_key.toString());
        }
        settings.endGroup();
    }

    void setup_shortcuts() {
        // Delete existing shortcut objects
        for (auto &cfg : shortcut_configs_) {
            if (cfg.shortcut_obj) {
                delete cfg.shortcut_obj;
                cfg.shortcut_obj = nullptr;
            }
        }
        
        // Create new ones
        for (auto &cfg : shortcut_configs_) {
            if (!cfg.current_key.isEmpty()) {
                cfg.shortcut_obj = new QShortcut(cfg.current_key, this);
                QString action_id = cfg.action_id;
                connect(cfg.shortcut_obj, &QShortcut::activated, this, [this, action_id]() {
                    trigger_shortcut_action(action_id);
                });
            }
        }
    }

    void trigger_shortcut_action(const QString &action_id) {
        if (action_id == "start") {
            if (start_button_->isEnabled()) {
                start_monitoring();
            }
        } else if (action_id == "stop") {
            if (stop_button_->isEnabled()) {
                stop_monitoring();
            }
        } else if (action_id == "add_image") {
            if (add_image_button_->isEnabled()) {
                add_clipboard_image();
            }
        } else if (action_id == "new_subject") {
            if (add_subject_button_->isEnabled()) {
                add_subject();
            }
        } else if (action_id == "open_note") {
            if (open_file_button_->isEnabled()) {
                open_selected_file();
            }
        } else if (action_id == "append") {
            if (append_to_heading_button_->isEnabled()) {
                manual_append_to_heading();
            }
        } else if (action_id == "inject") {
            if (inject_heading_button_->isEnabled()) {
                inject_heading_from_clipboard();
            }
        } else if (action_id == "shift") {
            if (shift_heading_button_->isEnabled()) {
                shift_selected_heading_section();
            }
        } else if (action_id == "delete") {
            if (delete_heading_button_->isEnabled()) {
                delete_selected_heading_section();
            }
        } else if (action_id == "new_section") {
            if (add_section_button_->isEnabled()) {
                add_section();
            }
        } else if (action_id == "toggle_format") {
            if (format_dropdown_->isEnabled() && format_dropdown_->count() > 0) {
                int next_idx = (format_dropdown_->currentIndex() + 1) % format_dropdown_->count();
                format_dropdown_->setCurrentIndex(next_idx);
                last_captured_label_->setText("ফরম্যাট পরিবর্তন করা হয়েছে: " + format_dropdown_->currentText());
            }
        } else if (action_id == "toggle_section") {
            if (section_dropdown_->isEnabled() && section_dropdown_->count() > 0) {
                int next_idx = (section_dropdown_->currentIndex() + 1) % section_dropdown_->count();
                section_dropdown_->setCurrentIndex(next_idx);
                last_captured_label_->setText("বিভাগ পরিবর্তন করা হয়েছে: " + section_dropdown_->currentText());
            }
        }
    }

    void open_settings_dialog() {
        ShortcutsSettingsDialog dlg(shortcut_configs_, this);
        if (dlg.exec() == QDialog::Accepted) {
            save_settings();
            setup_shortcuts();
            status_label_->setText("অবস্থা: শর্টকাটসমূহ সফলভাবে সংরক্ষণ করা হয়েছে!");
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Global UI Styling to ensure black text on light backgrounds
    app.setStyleSheet(
        "QWidget { background-color: #f5f6fa; font-family: 'Segoe UI', 'Kalpurush'; color: #2f3640; }"
        "QFrame#card { background-color: white; border: 1px solid #dcdde1; border-radius: 8px; }"
        "QLabel { color: #2f3640; font-size: 14px; }"
        "QPushButton { background-color: #487eb0; color: white; border-radius: 5px; padding: 10px; font-weight: bold; border: none; min-width: 80px; }"
        "QPushButton:hover { background-color: #40739e; }"
        "QPushButton:disabled { background-color: #dcdde1; color: #7f8c8d; }"
        "QComboBox { border: 1px solid #dcdde1; border-radius: 4px; padding: 5px; background: white; color: black; min-height: 30px; }"
        "QComboBox QAbstractItemView { background: white; color: black; selection-background-color: #487eb0; selection-color: white; }"
        "QLineEdit { background: white; color: black; padding: 5px; border: 1px solid #dcdde1; border-radius: 4px; }"
        "#status_label { font-size: 15px; font-weight: bold; color: #192a56; padding: 5px; background: transparent; }"
        "#last_captured_label { background-color: white; border: 1px solid #dcdde1; border-radius: 5px; padding: 12px; color: #2f3640; font-style: italic; border-left: 4px solid #487eb0; }"
    );

    // Add the Kalpurush font
    int fontId = QFontDatabase::addApplicationFont(":/Kalpurush.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QFont font(fontFamilies.at(0));
            app.setFont(font);
        }
    }

    ClipboardGrabber window;
    window.show();

    return app.exec();
}

#include "main.moc"
