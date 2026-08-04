#include "ClipboardGrabber.h"
#include "ActionRegistry.h"
#include "ServiceRegistry.h"
#include "FeatureManager.h"
#include "dialogs/ExportNoteWizard.h"

#include <QKeySequence>
#include <QVariantMap>

void ClipboardGrabber::setup_services() {
    auto note_svc = std::shared_ptr<INoteService>(&note_service_, [](INoteService*){});
    ServiceRegistry::instance().registerService<INoteService>(note_svc);

    auto clip_mon = std::shared_ptr<IClipboardMonitor>(clipboard_monitor_, [](IClipboardMonitor*){});
    ServiceRegistry::instance().registerService<IClipboardMonitor>(clip_mon);
}

void ClipboardGrabber::setup_features() {
    FeatureManager::instance().registerFeature(std::make_shared<ExportNoteWizard>(this));
}

void ClipboardGrabber::setup_actions() {
    ActionRegistry &reg = ActionRegistry::instance();

    reg.registerFunctionalAction("start", "শুরু করুন (Start)", "ক্লিপবোর্ড ট্র্যাকিং চালু করুন",
        "Monitoring", QKeySequence("Ctrl+Shift+S"),
        [this](const QVariantMap &) { start_monitoring(); },
        [this]() { return !is_running_ && ui_.subject_dropdown->currentIndex() != -1; });

    reg.registerFunctionalAction("stop", "থামুন (Stop)", "ক্লিপবোর্ড ট্র্যাকিং বন্ধ করুন",
        "Monitoring", QKeySequence("Ctrl+Shift+T"),
        [this](const QVariantMap &) { stop_monitoring(); },
        [this]() { return is_running_; });

    reg.registerFunctionalAction("add_image", "ছবি যুক্ত করুন (Add Image)", "ক্লিপবোর্ডের ছবি যুক্ত করুন",
        "Capture", QKeySequence("Ctrl+Shift+I"),
        [this](const QVariantMap &) { add_clipboard_image(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1; });

    reg.registerFunctionalAction("new_subject", "নতুন বিষয় (New Subject)", "নতুন বিষয় তৈরি করুন",
        "Subject", QKeySequence("Ctrl+Shift+N"),
        [this](const QVariantMap &) { add_subject(); });

    reg.registerFunctionalAction("add_folder", "নতুন ফোল্ডার (New Folder)", "নতুন ফোল্ডার তৈরি করুন",
        "Subject", QKeySequence("Ctrl+Shift+B"),
        [this](const QVariantMap &) { add_folder(); });

    reg.registerFunctionalAction("open_note", "নোট খুলুন (Open Note)", "সক্রিয় নোট ফাইল ওপেন করুন",
        "File", QKeySequence("Ctrl+Shift+O"),
        [this](const QVariantMap &) { open_selected_file(); },
        [this]() {
            return ui_.subject_dropdown->currentIndex() != -1
                && ui_.subject_dropdown->currentText() != "নির্বাচিত নয়";
        });

    reg.registerFunctionalAction("append", "যুক্ত করুন (Append)", "টার্গেট শিরোনামে কনটেন্ট যুক্ত করুন",
        "Heading", QKeySequence("Ctrl+Shift+A"),
        [this](const QVariantMap &) { manual_append_to_heading(); },
        [this]() {
            return ui_.subject_dropdown->currentIndex() != -1
                && !selected_heading_slug_.isEmpty();
        });

    reg.registerFunctionalAction("inject", "ইনজেক্ট করুন (Inject)",
        "ক্লিপবোর্ডের লেখা নতুন শিরোনাম হিসেবে যুক্ত করুন",
        "Heading", QKeySequence("Ctrl+Shift+J"),
        [this](const QVariantMap &) { inject_heading_from_clipboard(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1; });

    reg.registerFunctionalAction("select_heading", "শিরোনাম নির্বাচন (Select Heading)",
        "টার্গেট শিরোনাম খুঁজুন ও নির্বাচন করুন",
        "Heading", QKeySequence("Ctrl+Shift+G"),
        [this](const QVariantMap &) { open_heading_select_dialog(); },
        [this]() { return ui_.subject_dropdown->currentIndex() != -1; });

    reg.registerFunctionalAction("shift", "স্থানান্তর (Shift)", "শিরোনাম অন্য স্থানে স্থানান্তর করুন",
        "Heading", QKeySequence("Ctrl+Shift+H"),
        [this](const QVariantMap &) { shift_selected_heading_section(); },
        [this]() {
            return ui_.subject_dropdown->currentIndex() != -1
                && !selected_heading_slug_.isEmpty();
        });

    reg.registerFunctionalAction("delete", "মুছে ফেলুন (Delete)", "টার্গেট শিরোনাম সেকশন ডিলিট করুন",
        "Heading", QKeySequence("Ctrl+Shift+D"),
        [this](const QVariantMap &) { delete_selected_heading_section(); },
        [this]() {
            return ui_.subject_dropdown->currentIndex() != -1
                && !selected_heading_slug_.isEmpty();
        });

    reg.registerFunctionalAction("new_section", "নতুন বিভাগ (New Section)", "কাস্টম বিভাগ যোগ করুন",
        "Section", QKeySequence("Ctrl+Shift+K"),
        [this](const QVariantMap &) { add_section(); });

    reg.registerFunctionalAction("insert_diagram", "নতুন ডায়াগ্রাম (New Diagram)",
        "বর্তমান ডায়াগ্রাম শেষ করে নতুন রুট নোড দিয়ে আবার শুরু করুন",
        "Diagram", QKeySequence("Ctrl+Shift+M"),
        [this](const QVariantMap &) { insert_diagram(); },
        [this]() {
            return diagram_panel_enabled_ && is_diagram_format_selected()
                && ui_.subject_dropdown->currentIndex() != -1;
        });

    reg.registerFunctionalAction("toggle_format", "ফরম্যাট পরিবর্তন (Toggle Format)",
        "ক্যাপচার ফরম্যাট সাইকেল করুন", "Capture", QKeySequence("Ctrl+Shift+F"),
        [this](const QVariantMap &) {
            if (ui_.format_dropdown->isEnabled() && ui_.format_dropdown->count() > 0) {
                int next = (ui_.format_dropdown->currentIndex() + 1) % ui_.format_dropdown->count();
                ui_.format_dropdown->setCurrentIndex(next);
                ui_.last_captured_label->setText(
                    "ফরম্যাট পরিবর্তন করা হয়েছে: " + ui_.format_dropdown->currentText());
            }
        });

    reg.registerFunctionalAction("toggle_section", "বিভাগ পরিবর্তন (Toggle Section)",
        "বিভাগ ক্যাটগরি সাইকেল করুন", "Section", QKeySequence("Ctrl+Shift+C"),
        [this](const QVariantMap &) {
            if (ui_.section_dropdown->isEnabled() && ui_.section_dropdown->count() > 0) {
                int next = (ui_.section_dropdown->currentIndex() + 1) % ui_.section_dropdown->count();
                ui_.section_dropdown->setCurrentIndex(next);
                ui_.last_captured_label->setText(
                    "বিভাগ পরিবর্তন করা হয়েছে: " + ui_.section_dropdown->currentText());
            }
        });

    reg.registerFunctionalAction("toggle_subject", "বিষয় পরিবর্তন (Toggle Subject)",
        "পরবর্তী বিষয় নির্বাচন করুন", "Subject", QKeySequence("Ctrl+Shift+E"),
        [this](const QVariantMap &) { toggle_subject(); });

    reg.registerFunctionalAction("toggle_folder", "ফোল্ডার পরিবর্তন (Toggle Folder)",
        "পরবর্তী ফোল্ডার নির্বাচন করুন", "Subject", QKeySequence("Ctrl+Shift+U"),
        [this](const QVariantMap &) {
            if (ui_.folder_dropdown->isEnabled() && ui_.folder_dropdown->count() > 0) {
                int next = (ui_.folder_dropdown->currentIndex() + 1) % ui_.folder_dropdown->count();
                ui_.folder_dropdown->setCurrentIndex(next);
                ui_.last_captured_label->setText(
                    "ফোল্ডার পরিবর্তন করা হয়েছে: " + ui_.folder_dropdown->currentText());
            }
        },
        [this]() { return ui_.folder_dropdown->isEnabled() && ui_.folder_dropdown->count() > 0; });

    reg.registerFunctionalAction("toggle_mode", "মোড পরিবর্তন (Toggle Mode)",
        "কপি / সিলেক্ট মোড সাইকেল করুন", "Capture", QKeySequence("Ctrl+Shift+W"),
        [this](const QVariantMap &) {
            if (ui_.mode_dropdown->isEnabled() && ui_.mode_dropdown->count() > 0) {
                int next = (ui_.mode_dropdown->currentIndex() + 1) % ui_.mode_dropdown->count();
                ui_.mode_dropdown->setCurrentIndex(next);
                ui_.last_captured_label->setText(
                    "মোড পরিবর্তন করা হয়েছে: " + ui_.mode_dropdown->currentText());
            }
        },
        [this]() { return ui_.mode_dropdown->isEnabled() && ui_.mode_dropdown->count() > 0; });

    reg.registerFunctionalAction("toggle_diagram", "ডায়াগ্রাম টেমপ্লেট পরিবর্তন (Toggle Diagram Template)",
        "ডায়াগ্রাম টেমপ্লেট সাইকেল করুন", "Diagram", QKeySequence("Ctrl+Shift+L"),
        [this](const QVariantMap &) {
            if (ui_.diagram_dropdown->isEnabled() && ui_.diagram_dropdown->count() > 0) {
                int next = (ui_.diagram_dropdown->currentIndex() + 1) % ui_.diagram_dropdown->count();
                ui_.diagram_dropdown->setCurrentIndex(next);
                ui_.last_captured_label->setText(
                    "ডায়াগ্রাম টেমপ্লেট পরিবর্তন করা হয়েছে: " + ui_.diagram_dropdown->currentText());
            }
        },
        [this]() { return ui_.diagram_dropdown->isEnabled() && ui_.diagram_dropdown->count() > 0; });

    reg.registerFunctionalAction("settings", "সেটিংস (Settings)", "শর্টকাট সেটিংস খুলুন",
        "System", QKeySequence("Ctrl+Shift+P"),
        [this](const QVariantMap &) { open_settings_dialog(); });

    reg.registerFunctionalAction("wizards", "উইজার্ড ও টুলস (Wizards & Tools)",
        "এক্সটেনশন ও উইজার্ড তালিকা খুলুন", "Extensions", QKeySequence(),
        [this](const QVariantMap &) { open_wizards_dialog(); });

    reg.registerFunctionalAction("always_on_top", "সর্বদা উপরে (Always On Top)",
        "উইন্ডো সর্বদা সবার উপরে পিন করুন/সরান", "System", QKeySequence("Ctrl+Shift+Y"),
        [this](const QVariantMap &) { toggle_always_on_top(); });
}
