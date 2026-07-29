#include "ClipboardGrabber.h"
#include "Utils.h"
#include "MarkdownUtils.h"
#include "ShortcutsSettingsDialog.h"
#include "HeadingSelectDialog.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QInputDialog>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>

ClipboardGrabber::ClipboardGrabber(QWidget *parent) : QWidget(parent) {
    // --- Setup UI ---
    ui_.setupUi(this);

    // --- State Variables ---
    is_running_ = false;
    last_date_ = "";

    // --- Modular Component Initialization ---
    clipboard_monitor_ = new ClipboardMonitor(this);
    connect(clipboard_monitor_, &IClipboardMonitor::textCaptured, this, &ClipboardGrabber::handle_text_captured);
    connect(clipboard_monitor_, &IClipboardMonitor::imageCaptured, this, &ClipboardGrabber::handle_image_captured);

    connect(&shortcut_manager_, &ShortcutManager::actionTriggered, this, &ClipboardGrabber::trigger_shortcut_action);
    QString settings_file_path = note_service_.notesDirPath() + QDir::separator() + "settings.ini";
    shortcut_manager_.loadSettings(settings_file_path);
    shortcut_manager_.setupShortcuts(this);

    load_sections_for_subject("");

    // --- Connections (Signals and Slots) ---
    connect(ui_.start_button, &QPushButton::clicked, this, [this]() { this->start_monitoring(); });
    connect(ui_.stop_button, &QPushButton::clicked, this, [this]() { this->stop_monitoring(); });
    connect(ui_.add_image_button, &QPushButton::clicked, this, [this]() { this->add_clipboard_image(); });
    connect(ui_.toggle_subject_button, &QPushButton::clicked, this, [this]() { this->toggle_subject(); });
    connect(ui_.add_subject_button, &QPushButton::clicked, this, [this]() { this->add_subject(); });
    connect(ui_.add_folder_button, &QPushButton::clicked, this, [this]() { this->add_folder(); });
    connect(ui_.open_file_button, &QPushButton::clicked, this, [this]() { this->open_selected_file(); });
    connect(ui_.subject_dropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) { this->on_subject_changed(text); });
    connect(ui_.select_heading_button, &QPushButton::clicked, this, [this]() { this->open_heading_select_dialog(); });
    connect(ui_.append_to_heading_button, &QPushButton::clicked, this, [this]() { this->manual_append_to_heading(); });
    connect(ui_.delete_heading_button, &QPushButton::clicked, this, [this]() { this->delete_selected_heading_section(); });
    connect(ui_.inject_heading_button, &QPushButton::clicked, this, [this]() { this->inject_heading_from_clipboard(); });
    connect(ui_.shift_heading_button, &QPushButton::clicked, this, [this]() { this->shift_selected_heading_section(); });
    connect(ui_.add_section_button, &QPushButton::clicked, this, [this]() { this->add_section(); });
    connect(ui_.settings_button, &QPushButton::clicked, this, [this]() { this->open_settings_dialog(); });

    // --- Initial Population ---
    populate_subjects_from_disk();
    ui_.subject_dropdown->setCurrentIndex(-1); // No initial selection
    update_status_label();
}

void ClipboardGrabber::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
}

QList<SectionItem> ClipboardGrabber::get_sections_from_ui() const {
    QList<SectionItem> sections;
    for (int i = 0; i < ui_.section_dropdown->count(); ++i) {
        sections.append({ui_.section_dropdown->itemText(i), ui_.section_dropdown->itemData(i).toString()});
    }
    return sections;
}

void ClipboardGrabber::populate_sections_ui(const QList<SectionItem> &sections) {
    ui_.section_dropdown->blockSignals(true);
    ui_.section_dropdown->clear();
    for (const auto &sec : sections) {
        ui_.section_dropdown->addItem(sec.displayName, sec.slug);
    }
    ui_.section_dropdown->blockSignals(false);
}

void ClipboardGrabber::toggle_subject() {
    if (ui_.subject_dropdown->isEnabled() && ui_.subject_dropdown->count() > 0) {
        int count = ui_.subject_dropdown->count();
        int current = ui_.subject_dropdown->currentIndex();
        int next_idx = (current + 1) % count;
        ui_.subject_dropdown->setCurrentIndex(next_idx);
    }
}

void ClipboardGrabber::start_monitoring() {
    if (ui_.subject_dropdown->currentIndex() == -1) {
        ui_.status_label->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
        return;
    }
    is_running_ = true;
    
    last_date_ = MarkdownUtils::restore_state_from_file(get_current_target_file());
    note_service_.updateTocInFile(get_current_target_file(), get_sections_from_ui());
    
    QClipboard::Mode mode = (ui_.mode_dropdown->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
    clipboard_monitor_->start(mode, 1000);
    
    ui_.start_button->setEnabled(false);
    ui_.stop_button->setEnabled(true);
    ui_.subject_dropdown->setEnabled(false);
    ui_.toggle_subject_button->setEnabled(false);
    ui_.add_subject_button->setEnabled(false);
    ui_.add_folder_button->setEnabled(false);
    ui_.mode_dropdown->setEnabled(false);
    update_status_label();
}

void ClipboardGrabber::stop_monitoring() {
    is_running_ = false;
    clipboard_monitor_->stop();
    note_service_.updateTocInFile(get_current_target_file(), get_sections_from_ui());
    ui_.start_button->setEnabled(true);
    ui_.stop_button->setEnabled(false);
    ui_.subject_dropdown->setEnabled(true);
    ui_.toggle_subject_button->setEnabled(true);
    ui_.add_subject_button->setEnabled(true);
    ui_.add_folder_button->setEnabled(true);
    ui_.mode_dropdown->setEnabled(true);
    update_status_label();
}

void ClipboardGrabber::add_subject() {
    bool ok;
    QString text = QInputDialog::getText(this, "বিষয় যোগ করুন",
                                         "নতুন বিষয়ের নাম (ফোল্ডার সহ, যেমন: BCS/Bangla):", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        if (ui_.subject_dropdown->findText(text) == -1) {
            ui_.subject_dropdown->addItem(text);
            note_service_.createSubject(text);
        }
        ui_.subject_dropdown->setCurrentText(text);
    }
}

void ClipboardGrabber::add_folder() {
    bool ok;
    QString text = QInputDialog::getText(this, "ফোল্ডার তৈরি করুন",
                                         "নতুন ফোল্ডারের নাম (Path):", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        QString status_msg;
        note_service_.createFolder(text, status_msg);
        ui_.status_label->setText(status_msg);
    }
}

void ClipboardGrabber::handle_text_captured(const QString &text) {
    ui_.last_captured_label->setText("শেষ ক্যাপচার: " + text);
    QString current_section = ui_.section_dropdown->currentData().toString();
    write_to_file(text, current_section);
}

void ClipboardGrabber::handle_image_captured(const QImage &image) {
    Q_UNUSED(image);
}

void ClipboardGrabber::save_sections_for_subject(const QString &subject_name) {
    note_service_.saveSectionsForSubject(subject_name, get_sections_from_ui());
}

void ClipboardGrabber::load_sections_for_subject(const QString &subject_name) {
    QList<SectionItem> sections = note_service_.loadSectionsForSubject(subject_name);
    populate_sections_ui(sections);
}

void ClipboardGrabber::on_subject_changed(const QString &text) {
    selected_heading_slug_ = "";
    selected_heading_title_ = "";
    ui_.select_heading_button->setText("(শেষে নতুন করে যোগ করুন / Append to End)");
    load_sections_for_subject(text);
    populate_headings_from_file();
}

void ClipboardGrabber::inject_heading_from_clipboard() {
    QClipboard::Mode mode = (ui_.mode_dropdown->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
    QString current_text = QGuiApplication::clipboard()->text(mode);
    QString simplified_text = current_text.simplified();

    if (simplified_text.isEmpty()) {
        ui_.last_captured_label->setText("ইনজেক্ট ত্রুটি: ক্লিপবোর্ডে কোনো টেক্সট নেই!");
        return;
    }

    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") return;

    QString current_section = ui_.section_dropdown->currentData().toString();
    if (note_service_.injectHeadingToNote(target_file, simplified_text, current_section, last_date_)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText("ইনজেক্ট করা হয়েছে: " + simplified_text);
        populate_headings_from_file();
    } else {
        ui_.last_captured_label->setText("ইনজেক্ট ত্রুটি: ফাইলে লেখা যায়নি!");
    }
}

void ClipboardGrabber::open_selected_file() {
    QString target_file = get_current_target_file();
    if (target_file != "নির্বাচিত নয়" && QFile::exists(target_file)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(target_file));
    }
}

void ClipboardGrabber::add_clipboard_image() {
    QImage image = QGuiApplication::clipboard()->image();
    if (image.isNull()) {
        ui_.last_captured_label->setText("ছবি যুক্ত করার ত্রুটি: ক্লিপবোর্ডে কোনো ছবি নেই!");
        return;
    }

    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") return;

    QFileInfo file_info(target_file);
    QDir target_dir = file_info.dir();
    QString images_dir_path = target_dir.filePath("images");
    QDir images_dir(images_dir_path);
    if (!images_dir.exists()) {
        images_dir.mkpath(".");
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString image_filename = "img_" + timestamp + ".png";
    QString image_filepath = images_dir.filePath(image_filename);

    if (image.save(image_filepath, "PNG")) {
        note_service_.writeImageToNote(target_file, image_filename, last_date_);
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText("ছবি সফলভাবে যুক্ত করা হয়েছে: " + image_filename);
    } else {
        ui_.last_captured_label->setText("ছবি সংরক্ষণ করতে ব্যর্থ!");
    }
}

void ClipboardGrabber::update_button_states() {
    bool has_subject = (ui_.subject_dropdown->currentIndex() != -1 && ui_.subject_dropdown->currentText() != "নির্বাচিত নয়");
    ui_.open_file_button->setEnabled(has_subject);
    ui_.select_heading_button->setEnabled(has_subject);
    ui_.inject_heading_button->setEnabled(has_subject);
    bool has_heading = has_subject && !selected_heading_slug_.isEmpty();
    ui_.append_to_heading_button->setEnabled(has_heading);
    ui_.delete_heading_button->setEnabled(has_heading);
    ui_.shift_heading_button->setEnabled(has_heading);
}

void ClipboardGrabber::open_heading_select_dialog() {
    HeadingSelectDialog dlg(all_headings_, selected_heading_slug_, this);
    if (dlg.exec() == QDialog::Accepted) {
        selected_heading_slug_ = dlg.get_selected_slug();
        selected_heading_title_ = dlg.get_selected_title();
        if (selected_heading_slug_.isEmpty()) {
            ui_.select_heading_button->setText("(শেষে নতুন করে যোগ করুন / Append to End)");
        } else {
            ui_.select_heading_button->setText(selected_heading_title_);
        }
    }
}

void ClipboardGrabber::add_section() {
    bool ok;
    QString text = QInputDialog::getText(this, "নতুন বিভাগ যোগ করুন",
                                         "নতুন বিভাগের নাম (যেমন: আইন ও সংবিধান / Law-Constitution):", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        QString display_name = text.trimmed();
        QString slug = QString::fromStdString(MarkdownUtils::generate_slug(display_name));
        
        bool exists = false;
        for (int i = 0; i < ui_.section_dropdown->count(); ++i) {
            if (ui_.section_dropdown->itemData(i).toString() == slug) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            ui_.section_dropdown->addItem(display_name, slug);
            custom_added_sections_.insert(slug);
            save_sections_for_subject(ui_.subject_dropdown->currentText());
        }
        ui_.section_dropdown->setCurrentIndex(ui_.section_dropdown->findData(slug));
    }
}

void ClipboardGrabber::shift_selected_heading_section() {
    if (ui_.subject_dropdown->currentIndex() == -1) return;
    if (selected_heading_slug_.isEmpty()) return;

    QString source_slug = selected_heading_slug_;
    QString target_file = get_current_target_file();

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

    bool ok;
    QString target_selection = QInputDialog::getItem(this, "সেকশন স্থানান্তর", 
                                                     "কোথায় স্থানান্তর করতে চান তা নির্বাচন করুন:", 
                                                     target_titles, 0, false, &ok);
    if (!ok) return;

    int selected_idx = target_titles.indexOf(target_selection);
    if (selected_idx == -1) return;

    QString target_slug = target_slugs.at(selected_idx);
    QString out_label_text;

    if (note_service_.shiftHeadingSection(target_file, source_slug, target_slug, all_headings_, out_label_text)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText(out_label_text);
        populate_headings_from_file();
    } else {
        ui_.last_captured_label->setText(out_label_text);
    }
}

void ClipboardGrabber::manual_append_to_heading() {
    if (ui_.subject_dropdown->currentIndex() == -1) return;
    if (selected_heading_slug_.isEmpty()) return;
    
    QClipboard *clipboard = QGuiApplication::clipboard();
    QClipboard::Mode mode = (ui_.mode_dropdown->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
    QString current_text = clipboard->text(mode);
    QString simplified_text = current_text.simplified();

    if (simplified_text.isEmpty()) {
        ui_.last_captured_label->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো লেখা পাওয়া যায়নি।");
        return;
    }

    QString target_file = get_current_target_file();
    QString slug = selected_heading_slug_;
    
    if (note_service_.appendContentToHeading(target_file, slug, simplified_text, ui_.format_dropdown->currentIndex(), ui_.section_dropdown->currentData().toString())) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText("শেষ ক্যাপচার (নির্বাচিত শিরোনামে ম্যানুয়ালি যুক্ত করা হয়েছে): " + simplified_text);
    } else {
        ui_.last_captured_label->setText("ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!");
    }
}

void ClipboardGrabber::delete_selected_heading_section() {
    if (ui_.subject_dropdown->currentIndex() == -1) return;
    if (selected_heading_slug_.isEmpty()) return;
    
    QString slug = selected_heading_slug_;
    QString target_file = get_current_target_file();
    QString out_label_text;

    if (note_service_.deleteHeadingSection(target_file, slug, ui_.subject_dropdown->currentText(), out_label_text)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText(out_label_text);
        selected_heading_slug_ = "";
        selected_heading_title_ = "";
        ui_.select_heading_button->setText("(শেষে নতুন করে যোগ করুন / Append to End)");
        populate_headings_from_file();
    } else {
        ui_.last_captured_label->setText(out_label_text);
    }
}

void ClipboardGrabber::populate_subjects_from_disk() {
    QString current = ui_.subject_dropdown->currentText();
    ui_.subject_dropdown->blockSignals(true);
    ui_.subject_dropdown->clear();
    QStringList subjects = note_service_.populateSubjects(get_sections_from_ui());
    ui_.subject_dropdown->addItems(subjects);
    if (!current.isEmpty() && subjects.contains(current)) {
        ui_.subject_dropdown->setCurrentText(current);
    } else if (ui_.subject_dropdown->count() > 0) {
        ui_.subject_dropdown->setCurrentIndex(0);
    }
    ui_.subject_dropdown->blockSignals(false);
    on_subject_changed(ui_.subject_dropdown->currentText());
}

QString ClipboardGrabber::get_current_target_file() {
    return note_service_.getTargetFilePath(ui_.subject_dropdown->currentText());
}

void ClipboardGrabber::update_status_label() {
    if (is_running_) {
        ui_.status_label->setText("অবস্থা: ক্লিপবোর্ড পর্যবেক্ষণ চালুরত...");
    } else {
        ui_.status_label->setText("অবস্থা: বন্ধ");
    }
}

void ClipboardGrabber::write_to_file(const QString &processed_text, const QString &section) {
    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") return;

    QString out_captured_label_text;
    int format_index = ui_.format_dropdown->currentIndex();
    bool success = note_service_.writeToNote(target_file, processed_text, format_index, section, selected_heading_slug_, last_date_, out_captured_label_text);
    if (!out_captured_label_text.isEmpty()) {
        ui_.last_captured_label->setText(out_captured_label_text);
    }
    if (success) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        if (format_index == 1) {
            populate_headings_from_file();
        }
    }
}

void ClipboardGrabber::populate_headings_from_file() {
    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") {
        all_headings_.clear();
        update_button_states();
        return;
    }
    all_headings_ = note_service_.parseNoteStructure(target_file, get_sections_from_ui(), custom_added_sections_, ui_.subject_dropdown->currentText());
    update_button_states();
}

void ClipboardGrabber::trigger_shortcut_action(const QString &action_id) {
    if (action_id == "start") {
        if (ui_.start_button->isEnabled()) {
            start_monitoring();
        }
    } else if (action_id == "stop") {
        if (ui_.stop_button->isEnabled()) {
            stop_monitoring();
        }
    } else if (action_id == "add_image") {
        if (ui_.add_image_button->isEnabled()) {
            add_clipboard_image();
        }
    } else if (action_id == "new_subject") {
        if (ui_.add_subject_button->isEnabled()) {
            add_subject();
        }
    } else if (action_id == "open_note") {
        if (ui_.open_file_button->isEnabled()) {
            open_selected_file();
        }
    } else if (action_id == "append") {
        if (ui_.append_to_heading_button->isEnabled()) {
            manual_append_to_heading();
        }
    } else if (action_id == "inject") {
        if (ui_.inject_heading_button->isEnabled()) {
            inject_heading_from_clipboard();
        }
    } else if (action_id == "shift") {
        if (ui_.shift_heading_button->isEnabled()) {
            shift_selected_heading_section();
        }
    } else if (action_id == "delete") {
        if (ui_.delete_heading_button->isEnabled()) {
            delete_selected_heading_section();
        }
    } else if (action_id == "new_section") {
        if (ui_.add_section_button->isEnabled()) {
            add_section();
        }
    } else if (action_id == "toggle_format") {
        if (ui_.format_dropdown->isEnabled() && ui_.format_dropdown->count() > 0) {
            int next_idx = (ui_.format_dropdown->currentIndex() + 1) % ui_.format_dropdown->count();
            ui_.format_dropdown->setCurrentIndex(next_idx);
            ui_.last_captured_label->setText("ফরম্যাট পরিবর্তন করা হয়েছে: " + ui_.format_dropdown->currentText());
        }
    } else if (action_id == "toggle_section") {
        if (ui_.section_dropdown->isEnabled() && ui_.section_dropdown->count() > 0) {
            int next_idx = (ui_.section_dropdown->currentIndex() + 1) % ui_.section_dropdown->count();
            ui_.section_dropdown->setCurrentIndex(next_idx);
            ui_.last_captured_label->setText("বিভাগ পরিবর্তন করা হয়েছে: " + ui_.section_dropdown->currentText());
        }
    } else if (action_id == "toggle_subject") {
        if (ui_.toggle_subject_button->isEnabled()) {
            toggle_subject();
        }
    }
}

void ClipboardGrabber::open_settings_dialog() {
    ShortcutsSettingsDialog dlg(shortcut_manager_.configs(), this);
    if (dlg.exec() == QDialog::Accepted) {
        QString settings_file_path = note_service_.notesDirPath() + QDir::separator() + "settings.ini";
        shortcut_manager_.saveSettings(settings_file_path);
        shortcut_manager_.setupShortcuts(this);
        ui_.status_label->setText("অবস্থা: শর্টকাটসমূহ সফলভাবে সংরক্ষণ করা হয়েছে!");
    }
}
