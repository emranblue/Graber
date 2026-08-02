#include "ClipboardGrabber.h"
#include "Utils.h"
#include "MarkdownUtils.h"
#include "ShortcutsSettingsDialog.h"
#include "HeadingSelectDialog.h"
#include "ExportNoteWizard.h"
#include "ServiceRegistry.h"
#include "ActionRegistry.h"
#include "FeatureManager.h"

#include <QClipboard>
#include <QStyle>
#include <QGuiApplication>
#include <QInputDialog>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QScreen>
#include <QLayout>

ClipboardGrabber::ClipboardGrabber(QWidget *parent) : QWidget(parent) {
    // --- Setup UI ---
    ui_.setupUi(this);

    // --- State Variables ---
    is_running_ = false;
    is_always_on_top_ = false;
    last_date_ = "";

    // --- Modular Component Initialization ---
    clipboard_monitor_ = new ClipboardMonitor(this);
    connect(clipboard_monitor_, &IClipboardMonitor::textCaptured, this, &ClipboardGrabber::handle_text_captured);
    connect(clipboard_monitor_, &IClipboardMonitor::imageCaptured, this, &ClipboardGrabber::handle_image_captured);

    // --- Setup Abstraction Layers (Service, Action, Feature Registries) ---
    setup_services();
    setup_actions();
    setup_features();

    connect(&shortcut_manager_, &ShortcutManager::actionTriggered, this, &ClipboardGrabber::trigger_shortcut_action);
    QString settings_file_path = note_service_.notesDirPath() + QDir::separator() + "settings.ini";
    shortcut_manager_.loadSettings(settings_file_path);
    shortcut_manager_.setupShortcuts(this);

    load_sections_for_subject("");

    // --- Bind UI Buttons to Action Registry ---
    ActionRegistry::instance().bindButton(ui_.start_button, "start");
    ActionRegistry::instance().bindButton(ui_.stop_button, "stop");
    ActionRegistry::instance().bindButton(ui_.add_image_button, "add_image");
    ActionRegistry::instance().bindButton(ui_.toggle_subject_button, "toggle_subject");
    ActionRegistry::instance().bindButton(ui_.add_subject_button, "new_subject");
    ActionRegistry::instance().bindButton(ui_.add_folder_button, "add_folder");
    ActionRegistry::instance().bindButton(ui_.open_file_button, "open_note");
    ActionRegistry::instance().bindButton(ui_.append_to_heading_button, "append");
    ActionRegistry::instance().bindButton(ui_.delete_heading_button, "delete");
    ActionRegistry::instance().bindButton(ui_.inject_heading_button, "inject");
    ActionRegistry::instance().bindButton(ui_.shift_heading_button, "shift");
    ActionRegistry::instance().bindButton(ui_.add_section_button, "new_section");
    ActionRegistry::instance().bindButton(ui_.settings_button, "settings");
    ActionRegistry::instance().bindButton(ui_.wizards_button, "wizards");
    ActionRegistry::instance().bindButton(ui_.select_heading_button, "select_heading");

    connect(ui_.folder_dropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClipboardGrabber::on_folder_changed);
    connect(ui_.subject_dropdown, &QComboBox::currentTextChanged, this, [this](const QString &text) { this->on_subject_changed(text); });

    // --- Initial Population ---
    populate_folders_from_disk();
    populate_subjects_from_disk();
    ui_.subject_dropdown->setCurrentIndex(-1); // No initial selection
    update_status_label();

    // Size the window to what it actually contains instead of a guessed
    // constant, once the event loop has laid everything out.
    QTimer::singleShot(0, this, &ClipboardGrabber::fit_window_to_content);
}

void ClipboardGrabber::fit_window_to_content() {
    // Recompute layouts first so hints reflect whichever cards are currently
    // visible (full editing view or the collapsed capture view).
    layout()->invalidate();
    layout()->activate();
    ui_.body->layout()->invalidate();
    ui_.body->layout()->activate();

    // IMPORTANT: don't ask the top-level layout (or the QScrollArea inside
    // it) for its size hint. QScrollArea hardcodes sizeHint()/minimumSizeHint()
    // to a small capped value that ignores the widget scrolled inside it, so
    // going through it silently produced a window too small for the real
    // content no matter what the outer layout reported. Measuring body and
    // controls_bar directly gets the real numbers.
    const QSize body_hint = ui_.body->sizeHint();
    const QSize controls_hint = ui_.controls_bar->sizeHint();

    const QSize body_min = ui_.body->minimumSizeHint();
    const QSize controls_min = ui_.controls_bar->minimumSizeHint();

    int target_width = qMax(body_hint.width(), controls_hint.width());
    int target_height = body_hint.height() + controls_hint.height();

    int min_width = qMax(body_min.width(), controls_min.width());
    int min_height = body_min.height() + controls_min.height();

    setMinimumSize(qMax(min_width, 360), qMax(min_height, 360));

    // Never propose a window bigger than the screen can actually show.
    if (QScreen *scr = screen()) {
        const QRect avail = scr->availableGeometry();
        target_width = qMin(target_width, avail.width() - 40);
        target_height = qMin(target_height, avail.height() - 40);
    }

    target_width = qMax(target_width, minimumWidth());
    target_height = qMax(target_height, minimumHeight());

    resize(target_width, target_height);
}

void ClipboardGrabber::setup_services() {
    // Register NoteService in ServiceRegistry for loose coupling
    auto note_svc_ptr = std::shared_ptr<INoteService>(&note_service_, [](INoteService*){});
    ServiceRegistry::instance().registerService<INoteService>(note_svc_ptr);

    auto clip_mon_ptr = std::shared_ptr<IClipboardMonitor>(clipboard_monitor_, [](IClipboardMonitor*){});
    ServiceRegistry::instance().registerService<IClipboardMonitor>(clip_mon_ptr);
}

void ClipboardGrabber::setup_actions() {
    ActionRegistry &reg = ActionRegistry::instance();

    reg.registerFunctionalAction("start", "শুরু করুন (Start)", "ক্লিপবোর্ড ট্র্যাকিং চালু করুন", "Monitoring", QKeySequence("Ctrl+Shift+S"),
        [this](const QVariantMap &) { this->start_monitoring(); },
        [this]() { return !is_running_ && ui_.subject_dropdown->currentIndex() != -1; });

    reg.registerFunctionalAction("stop", "থামুন (Stop)", "ক্লিপবোর্ড ট্র্যাকিং বন্ধ করুন", "Monitoring", QKeySequence("Ctrl+Shift+T"),
        [this](const QVariantMap &) { this->stop_monitoring(); },
        [this]() { return is_running_; });

    reg.registerFunctionalAction("add_image", "ছবি যুক্ত করুন (Add Image)", "ক্লিপবোর্ডের ছবি যুক্ত করুন", "Capture", QKeySequence("Ctrl+Shift+I"),
        [this](const QVariantMap &) { this->add_clipboard_image(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1; });

    reg.registerFunctionalAction("new_subject", "নতুন বিষয় (New Subject)", "নতুন বিষয় তৈরি করুন", "Subject", QKeySequence("Ctrl+Shift+N"),
        [this](const QVariantMap &) { this->add_subject(); });

    reg.registerFunctionalAction("add_folder", "নতুন ফোল্ডার (New Folder)", "নতুন ফোল্ডার তৈরি করুন", "Subject", QKeySequence("Ctrl+Shift+B"),
        [this](const QVariantMap &) { this->add_folder(); });

    reg.registerFunctionalAction("open_note", "নোট খুলুন (Open Note)", "সক্রিয় নোট ফাইল ওপেন করুন", "File", QKeySequence("Ctrl+Shift+O"),
        [this](const QVariantMap &) { this->open_selected_file(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1 && ui_.subject_dropdown->currentText() != "নির্বাচিত নয়"; });

    reg.registerFunctionalAction("append", "যুক্ত করুন (Append)", "টার্গেট শিরোনামে কনটেন্ট যুক্ত করুন", "Heading", QKeySequence("Ctrl+Shift+A"),
        [this](const QVariantMap &) { this->manual_append_to_heading(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1 && !selected_heading_slug_.isEmpty(); });

    reg.registerFunctionalAction("inject", "ইনজেক্ট করুন (Inject)", "ক্লিপবোর্ডের লেখা নতুন শিরোনাম হিসেবে যুক্ত করুন", "Heading", QKeySequence("Ctrl+Shift+J"),
        [this](const QVariantMap &) { this->inject_heading_from_clipboard(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1; });

    reg.registerFunctionalAction("select_heading", "শিরোনাম নির্বাচন (Select Heading)", "টার্গেট শিরোনাম খুঁজুন ও নির্বাচন করুন", "Heading", QKeySequence("Ctrl+Shift+G"),
        [this](const QVariantMap &) { this->open_heading_select_dialog(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1; });

    reg.registerFunctionalAction("shift", "স্থানান্তর (Shift)", "শিরোনাম অন্য স্থানে স্থানান্তর করুন", "Heading", QKeySequence("Ctrl+Shift+H"),
        [this](const QVariantMap &) { this->shift_selected_heading_section(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1 && !selected_heading_slug_.isEmpty(); });

    reg.registerFunctionalAction("delete", "মুছে ফেলুন (Delete)", "টার্গেট শিরোনাম সেকশন ডিলিট করুন", "Heading", QKeySequence("Ctrl+Shift+D"),
        [this](const QVariantMap &) { this->delete_selected_heading_section(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1 && !selected_heading_slug_.isEmpty(); });

    reg.registerFunctionalAction("new_section", "নতুন বিভাগ (New Section)", "কাস্টম বিভাগ যোগ করুন", "Section", QKeySequence("Ctrl+Shift+K"),
        [this](const QVariantMap &) { this->add_section(); });

    reg.registerFunctionalAction("toggle_format", "ফরম্যাট পরিবর্তন (Toggle Format)", "ক্যাপচার ফরম্যাট সাইকেল করুন", "Capture", QKeySequence("Ctrl+Shift+F"),
        [this](const QVariantMap &) {
            if (ui_.format_dropdown->isEnabled() && ui_.format_dropdown->count() > 0) {
                int next_idx = (ui_.format_dropdown->currentIndex() + 1) % ui_.format_dropdown->count();
                ui_.format_dropdown->setCurrentIndex(next_idx);
                ui_.last_captured_label->setText("ফরম্যাট পরিবর্তন করা হয়েছে: " + ui_.format_dropdown->currentText());
            }
        });

    reg.registerFunctionalAction("toggle_section", "বিভাগ পরিবর্তন (Toggle Section)", "বিভাগ ক্যাটগরি সাইকেল করুন", "Section", QKeySequence("Ctrl+Shift+C"),
        [this](const QVariantMap &) {
            if (ui_.section_dropdown->isEnabled() && ui_.section_dropdown->count() > 0) {
                int next_idx = (ui_.section_dropdown->currentIndex() + 1) % ui_.section_dropdown->count();
                ui_.section_dropdown->setCurrentIndex(next_idx);
                ui_.last_captured_label->setText("বিভাগ পরিবর্তন করা হয়েছে: " + ui_.section_dropdown->currentText());
            }
        });

    reg.registerFunctionalAction("toggle_subject", "বিষয় পরিবর্তন (Toggle Subject)", "পরবর��তী বিষয় নির্বাচন করুন", "Subject", QKeySequence("Ctrl+Shift+E"),
        [this](const QVariantMap &) { this->toggle_subject(); });

    reg.registerFunctionalAction("settings", "সেটিংস (Settings)", "শর্টকাট সেটিংস খুলুন", "System", QKeySequence("Ctrl+Shift+P"),
        [this](const QVariantMap &) { this->open_settings_dialog(); });

    reg.registerFunctionalAction("wizards", "উইজার্ড ও টুলস (Wizards & Tools)", "এক্সটেনশন ও উইজার্ড তালিকা খুলুন", "Extensions", QKeySequence(),
        [this](const QVariantMap &) { this->open_wizards_dialog(); });

    reg.registerFunctionalAction("always_on_top", "সর্বদা উপরে (Always On Top)", "উইন্ডো সর্বদা সবার উপরে পিন করুন/সরান", "System", QKeySequence("Ctrl+Shift+Y"),
        [this](const QVariantMap &) { this->toggle_always_on_top(); });
}

void ClipboardGrabber::toggle_always_on_top() {
    is_always_on_top_ = !is_always_on_top_;

    // Changing window flags on a widget hides it on most platforms, so it
    // has to be shown again immediately afterwards to avoid a visible flicker
    // or the window disappearing behind others.
    setWindowFlag(Qt::WindowStaysOnTopHint, is_always_on_top_);
    show();

    ui_.status_label->setText(is_always_on_top_
        ? "অবস্থা: উইন্ডো সর্বদা উপরে পিন করা হয়েছে (Always On Top: ON)"
        : "অবস্থা: উইন্ডো স্বাভাবিক মোডে ফেরত এসেছে (Always On Top: OFF)");
}

void ClipboardGrabber::setup_features() {
    // Register built-in export note wizard
    FeatureManager::instance().registerFeature(std::make_shared<ExportNoteWizard>(this));
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
        ui_.status_label->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
        return;
    }
    is_running_ = true;
    
    last_date_ = MarkdownUtils::restore_state_from_file(get_current_target_file());
    note_service_.updateTocInFile(get_current_target_file(), get_sections_from_ui());
    
    QClipboard::Mode mode = (ui_.mode_dropdown->currentIndex() == 0) ? QClipboard::Clipboard : QClipboard::Selection;
    clipboard_monitor_->start(mode, 1000);
    
    ui_.subject_dropdown->setEnabled(false);
    ui_.folder_dropdown->setEnabled(false);
    ui_.add_folder_button->setEnabled(false);
    ui_.add_subject_button->setEnabled(false);
    ui_.mode_dropdown->setEnabled(false);
    update_status_label();
    ActionRegistry::instance().updateBoundButtons();

    // Focused capture view: hide folder/subject browsing, the extra section
    // & image controls, and heading management — leave just the status/log
    // card and the format selector visible while actively capturing.
    ui_.subject_card->setVisible(false);
    ui_.capture_extra->setVisible(false);
    ui_.heading_card->setVisible(false);

    // Shrink the window down to the now-smaller capture view instead of
    // leaving the old, larger size with empty space (or a scrollbar).
    QTimer::singleShot(0, this, &ClipboardGrabber::fit_window_to_content);
}

void ClipboardGrabber::stop_monitoring() {
    is_running_ = false;
    clipboard_monitor_->stop();
    note_service_.updateTocInFile(get_current_target_file(), get_sections_from_ui());
    ui_.subject_dropdown->setEnabled(true);
    ui_.folder_dropdown->setEnabled(true);
    ui_.add_folder_button->setEnabled(true);
    ui_.add_subject_button->setEnabled(true);
    ui_.mode_dropdown->setEnabled(true);
    update_status_label();
    ActionRegistry::instance().updateBoundButtons();

    // Back to the full editing view.
    ui_.subject_card->setVisible(true);
    ui_.capture_extra->setVisible(true);
    ui_.heading_card->setVisible(true);

    // Grow the window back to fit the full editing view again.
    QTimer::singleShot(0, this, &ClipboardGrabber::fit_window_to_content);
}

void ClipboardGrabber::add_subject() {
    bool ok;
    QString selected_folder = ui_.folder_dropdown->currentData().toString();
    QString default_prefix = "";
    if (!selected_folder.isEmpty() && selected_folder != "__ALL__" && selected_folder != "__ROOT__") {
        default_prefix = selected_folder + "/";
    }

    QString text = QInputDialog::getText(this, "বিষয় যোগ করুন",
                                         "নতুন বিষয়ের নাম (ফোল্ডার সহ, যেমন: BCS/Bangla):", QLineEdit::Normal,
                                         default_prefix, &ok);
    if (ok && !text.isEmpty()) {
        text = text.trimmed();
        text.replace('\\', '/');
        note_service_.createSubject(text);

        // If subject has folder hierarchy, refresh folders dropdown and set selection
        int last_slash = text.lastIndexOf('/');
        if (last_slash != -1) {
            populate_folders_from_disk();
            QString folder_part = text.left(last_slash);
            int f_idx = ui_.folder_dropdown->findData(folder_part);
            if (f_idx != -1) {
                ui_.folder_dropdown->blockSignals(true);
                ui_.folder_dropdown->setCurrentIndex(f_idx);
                ui_.folder_dropdown->blockSignals(false);
            }
        }

        populate_subjects_from_disk();
        int s_idx = ui_.subject_dropdown->findData(text);
        if (s_idx != -1) {
            ui_.subject_dropdown->setCurrentIndex(s_idx);
        }
    }
}

void ClipboardGrabber::add_folder() {
    bool ok;
    QString text = QInputDialog::getText(this, "ফোল্ডার তৈরি করুন",
                                         "নতুন ফোল্ডারের নাম (Path):", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        text = text.trimmed();
        text.replace('\\', '/');
        QString status_msg;
        if (note_service_.createFolder(text, status_msg)) {
            populate_folders_from_disk();
            int idx = ui_.folder_dropdown->findData(text);
            if (idx != -1) {
                ui_.folder_dropdown->setCurrentIndex(idx);
            } else {
                populate_subjects_from_disk();
            }
        }
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
    if (target_file == "নির্বাচিত নয়") return;

    QString current_section = ui_.section_dropdown->currentData().toString();
    if (note_service_.injectHeadingToNote(target_file, simplified_text, current_section, last_date_)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText("ইনজেক্ট করা হয়েছে: " + simplified_text);
        populate_headings_from_file();
    } else {
        ui_.last_captured_label->setText("ইনজেক্ট ত্রুটি: ফাইলে লেখা যায়নি!");
    }
}

void ClipboardGrabber::open_selected_file() {
    QString target_file = get_current_target_file();
    if (target_file != "নির্বাচিত নয়" && QFile::exists(target_file)) {
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
    if (target_file == "নির্বাচিত নয়") return;

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
        ui_.last_captured_label->setText("ছবি সফলভাবে যুক্ত করা হয়েছে: " + image_filename);
    } else {
        ui_.last_captured_label->setText("ছবি সংরক্ষণ করতে ব্যর্থ!");
    }
}

void ClipboardGrabber::update_button_states() {
    ActionRegistry::instance().updateBoundButtons();
}

void ClipboardGrabber::open_heading_select_dialog() {
    // Refresh the heading list from current file before opening dialog
    populate_headings_from_file();
    
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
    update_button_states();
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
            save_sections_for_subject(get_selected_subject_name());
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
                                                     "কোথায় স্থানান্তর করতে চান তা নির্বাচন করুন:", 
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
        ui_.last_captured_label->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো লেখা পাওয়া যায়নি।");
        return;
    }

    QString target_file = get_current_target_file();
    QString slug = selected_heading_slug_;
    
    if (note_service_.appendContentToHeading(target_file, slug, simplified_text, ui_.format_dropdown->currentIndex(), ui_.section_dropdown->currentData().toString())) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText("শেষ ক্যাপচার (নির্বাচিত শিরোনামে ম্যানুয়ালি যুক্ত করা হয়েছে): " + simplified_text);
    } else {
        ui_.last_captured_label->setText("ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!");
    }
}

void ClipboardGrabber::delete_selected_heading_section() {
    if (ui_.subject_dropdown->currentIndex() == -1) return;
    if (selected_heading_slug_.isEmpty()) return;
    
    QString slug = selected_heading_slug_;
    QString target_file = get_current_target_file();
    QString out_label_text;

    if (note_service_.deleteHeadingSection(target_file, slug, get_selected_subject_name(), out_label_text)) {
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

void ClipboardGrabber::on_folder_changed(int index) {
    Q_UNUSED(index);
    populate_subjects_from_disk();
}

void ClipboardGrabber::populate_folders_from_disk() {
    QString current_folder_data = ui_.folder_dropdown->currentData().toString();

    ui_.folder_dropdown->blockSignals(true);
    ui_.folder_dropdown->clear();

    static const QIcon folder_icon(":/icons/folder.ico");

    ui_.folder_dropdown->addItem(folder_icon, "সকল ফোল্ডার (All Folders)", "__ALL__");
    ui_.folder_dropdown->addItem(folder_icon, "রুট ফোল্ডার (Root / Base)", "__ROOT__");

    QStringList folders = note_service_.populateFolders();
    for (const QString &folder : folders) {
        ui_.folder_dropdown->addItem(folder_icon, folder, folder);
    }

    int found_idx = -1;
    if (!current_folder_data.isEmpty()) {
        found_idx = ui_.folder_dropdown->findData(current_folder_data);
    }
    if (found_idx != -1) {
        ui_.folder_dropdown->setCurrentIndex(found_idx);
    } else {
        ui_.folder_dropdown->setCurrentIndex(0);
    }

    ui_.folder_dropdown->blockSignals(false);
}

QString ClipboardGrabber::get_selected_subject_name() const {
    if (ui_.subject_dropdown->currentIndex() == -1) return "";
    QString data = ui_.subject_dropdown->currentData().toString();
    if (!data.isEmpty()) return data;
    return ui_.subject_dropdown->currentText();
}

void ClipboardGrabber::populate_subjects_from_disk() {
    QString current_full_name = get_selected_subject_name();
    QString folder_filter = ui_.folder_dropdown->currentData().toString();
    if (folder_filter.isEmpty()) {
        folder_filter = "__ALL__";
    }

    ui_.subject_dropdown->blockSignals(true);
    ui_.subject_dropdown->clear();

    static const QIcon file_icon(":/icons/file.ico");

    QList<SubjectItem> subject_items = note_service_.populateSubjectItems(get_sections_from_ui(), folder_filter);
    int select_idx = -1;
    for (int i = 0; i < subject_items.size(); ++i) {
        const auto &sub = subject_items.at(i);
        ui_.subject_dropdown->addItem(file_icon, sub.displayName, sub.fullPath);
        if (!current_full_name.isEmpty() && sub.fullPath == current_full_name) {
            select_idx = i;
        }
    }

    if (select_idx != -1) {
        ui_.subject_dropdown->setCurrentIndex(select_idx);
    } else if (ui_.subject_dropdown->count() > 0) {
        ui_.subject_dropdown->setCurrentIndex(0);
    } else {
        ui_.subject_dropdown->setCurrentIndex(-1);
    }

    ui_.subject_dropdown->blockSignals(false);
    on_subject_changed(get_selected_subject_name());
}

QString ClipboardGrabber::get_current_target_file() {
    return note_service_.getTargetFilePath(get_selected_subject_name());
}

void ClipboardGrabber::update_status_label() {
    if (is_running_) {
        ui_.status_label->setText("অবস্থা: ক্লিপবোর্ড পর্যবেক্ষণ চালুরত...");
    } else {
        ui_.status_label->setText("অবস্থা: বন্ধ");
    }
    ui_.status_label->setProperty("running", is_running_);
    ui_.status_label->style()->unpolish(ui_.status_label);
    ui_.status_label->style()->polish(ui_.status_label);
}

void ClipboardGrabber::write_to_file(const QString &processed_text, const QString &section) {
    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") return;

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
    if (target_file == "নির্বাচিত নয়") {
        all_headings_.clear();
        update_button_states();
        return;
    }
    all_headings_ = note_service_.parseNoteStructure(target_file, get_sections_from_ui(), custom_added_sections_, get_selected_subject_name());
    update_button_states();
}

void ClipboardGrabber::trigger_shortcut_action(const QString &action_id) {
    ActionRegistry::instance().executeAction(action_id);
}

void ClipboardGrabber::open_settings_dialog() {
    bool global_hotkeys_enabled = shortcut_manager_.globalHotkeysEnabled();
    ShortcutsSettingsDialog dlg(shortcut_manager_.configs(), global_hotkeys_enabled,
                                 shortcut_manager_.globalHotkeysSupported(), this);
    if (dlg.exec() == QDialog::Accepted) {
        shortcut_manager_.enableGlobalHotkeys(global_hotkeys_enabled);
        QString settings_file_path = note_service_.notesDirPath() + QDir::separator() + "settings.ini";
        shortcut_manager_.saveSettings(settings_file_path);
        shortcut_manager_.setupShortcuts(this);
        ui_.status_label->setText("অবস্থা: শর্টকাটসমূহ সফলভাবে সংরক্ষণ করা হয়েছে!");
    }
}

void ClipboardGrabber::open_wizards_dialog() {
    auto features = FeatureManager::instance().getAllFeatures();
    if (features.isEmpty()) {
        QMessageBox::information(this, "উইজার্ড ও টুলস", "কোনো এক্সটেনশন বা উইজার্ড ইনস্টল করা নেই।");
        return;
    }

    QMenu wizards_menu(this);
    for (const auto &feat : features) {
        QAction *act = wizards_menu.addAction(feat->displayName());
        connect(act, &QAction::triggered, this, [this, feat]() {
            feat->executeWizard(this, &ServiceRegistry::instance());
        });
    }
    wizards_menu.exec(ui_.wizards_button->mapToGlobal(QPoint(0, ui_.wizards_button->height())));
}
