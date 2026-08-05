#include "ClipboardGrabber.h"
#include "ActionRegistry.h"
#include "ServiceRegistry.h"
#include "FeatureManager.h"
#include "dialogs/HeadingSelectDialog.h"
#include "dialogs/ShortcutsSettingsDialog.h"
#include "MarkdownUtils.h"
#include "utils/UiAnimator.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QInputDialog>
#include <QSettings>
#include <QDir>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>

void ClipboardGrabber::populate_headings_from_file() {
    QString target = get_current_target_file();
    if (target == "নির্বাচিত নয়") {
        all_headings_.clear();
        update_button_states();
        return;
    }
    all_headings_ = note_service_.parseNoteStructure(
        target, get_sections_from_ui(), custom_added_sections_, get_selected_subject_name());
    update_button_states();
}

void ClipboardGrabber::open_heading_select_dialog() {
    populate_headings_from_file();

    HeadingSelectDialog dlg(all_headings_, selected_heading_slug_, this);
    if (UiAnimator::execDialogSmooth(&dlg) == QDialog::Accepted) {
        selected_heading_slug_ = dlg.get_selected_slug();
        selected_heading_title_ = dlg.get_selected_title();
        ui_.select_heading_button->setText(
            selected_heading_slug_.isEmpty()
                ? "(শেষে নতুন করে যোগ করুন / Append to End)"
                : selected_heading_title_);
    }
    update_button_states();
}

void ClipboardGrabber::inject_heading_from_clipboard() {
    QClipboard::Mode mode = (ui_.mode_dropdown->currentIndex() == 0)
        ? QClipboard::Clipboard : QClipboard::Selection;
    QString text = QGuiApplication::clipboard()->text(mode).simplified();
    if (text.isEmpty()) {
        ui_.last_captured_label->setText("ইনজেক্ট ত্রুটি: ক্লিপবোর্ডে কোনো টেক্সট নেই!");
        return;
    }

    QString target = get_current_target_file();
    if (target == "নির্বাচিত নয়") return;

    QString section = ui_.section_dropdown->currentData().toString();
    if (note_service_.injectHeadingToNote(target, text, section, last_date_)) {
        note_service_.updateTocInFile(target, get_sections_from_ui());
        ui_.last_captured_label->setText("ইনজেক্ট করা হয়েছে: " + text);
        populate_headings_from_file();
    } else {
        ui_.last_captured_label->setText("ইনজেক্ট ত্রুটি: ফাইলে লেখা যায়নি!");
    }
}

void ClipboardGrabber::manual_append_to_heading() {
    if (ui_.subject_dropdown->currentIndex() == -1 || selected_heading_slug_.isEmpty())
        return;

    QClipboard::Mode mode = (ui_.mode_dropdown->currentIndex() == 0)
        ? QClipboard::Clipboard : QClipboard::Selection;
    QString text = QGuiApplication::clipboard()->text(mode).simplified();
    if (text.isEmpty()) {
        ui_.last_captured_label->setText("শেষ ক্যাপচার: ক্লিপবোর্ডে কোনো লেখা পাওয়া যায়নি।");
        return;
    }

    QString target = get_current_target_file();
    if (note_service_.appendContentToHeading(target, selected_heading_slug_, text,
            ui_.format_dropdown->currentIndex(),
            ui_.section_dropdown->currentData().toString())) {
        note_service_.updateTocInFile(target, get_sections_from_ui());
        ui_.last_captured_label->setText(
            "শেষ ক্যাপচার (নির্বাচিত শিরোনামে ম্যানুয়ালি যুক্ত করা হয়েছে): " + text);
    } else {
        ui_.last_captured_label->setText("ত্রুটি: নির্বাচিত শিরোনামে যুক্ত করা যায়নি!");
    }
}

void ClipboardGrabber::shift_selected_heading_section() {
    if (ui_.subject_dropdown->currentIndex() == -1 || selected_heading_slug_.isEmpty())
        return;

    QString source = selected_heading_slug_;
    QString target_file = get_current_target_file();

    QStringList display_ids = MarkdownUtils::compute_display_ids(all_headings_);

    QStringList titles, slugs;
    titles << "(শেষে স্থানান্তর করুন / Move to End)";
    slugs << "";
    for (int i = 0; i < all_headings_.size(); ++i) {
        const NoteItem &item = all_headings_.at(i);
        if (item.slug == source) continue;
        const QString display_id = (i < display_ids.size()) ? display_ids.at(i) : QString();
        titles << (item.type == "heading"
            ? QString("%1 (id: %2)").arg(item.title, display_id)
            : QString("  ↳ %1 (id: %2)").arg(item.title, display_id));
        slugs << item.slug;
    }

    bool ok = false;
    QString choice = QInputDialog::getItem(this, "সেকশন স্থানান্তর",
        "কোথায় স্থানান্তর করতে চান তা নির্বাচন করুন:", titles, 0, false, &ok);
    if (!ok) return;

    int idx = titles.indexOf(choice);
    if (idx < 0) return;

    QString out;
    if (note_service_.shiftHeadingSection(target_file, source, slugs.at(idx),
                                          all_headings_, out)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        populate_headings_from_file();
    }
    ui_.last_captured_label->setText(out);
}

void ClipboardGrabber::delete_selected_heading_section() {
    if (ui_.subject_dropdown->currentIndex() == -1 || selected_heading_slug_.isEmpty())
        return;

    QString out;
    QString target = get_current_target_file();
    if (note_service_.deleteHeadingSection(target, selected_heading_slug_,
                                           get_selected_subject_name(), out)) {
        note_service_.updateTocInFile(target, get_sections_from_ui());
        selected_heading_slug_.clear();
        selected_heading_title_.clear();
        ui_.select_heading_button->setText("(শেষে নতুন করে যোগ করুন / Append to End)");
        populate_headings_from_file();
    }
    ui_.last_captured_label->setText(out);
}

