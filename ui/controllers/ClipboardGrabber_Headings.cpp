#include "ClipboardGrabber.h"
#include "ActionRegistry.h"
#include "ServiceRegistry.h"
#include "FeatureManager.h"
#include "dialogs/HeadingSelectDialog.h"
#include "dialogs/HeadingReorderDialog.h"
#include "dialogs/InjectFromNoteDialog.h"
#include "dialogs/ShortcutsSettingsDialog.h"
#include "MarkdownUtils.h"
#include "utils/UiAnimator.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QInputDialog>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QSet>

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
    // Inject = pick another GraberNotes .md → pick its subheading → copy block here.
    // Does NOT use clipboard.
    QString target = get_current_target_file();
    if (target == QStringLiteral("নির্বাচিত নয়") || target.isEmpty()) {
        ui_.last_captured_label->setText(
            QStringLiteral("ইনজেক্ট: আগে একটি টার্গেট নোট নির্বাচন করুন।"));
        return;
    }

    const QString notes_dir = note_service_.notesDirPath();
    InjectFromNoteDialog dlg(
        notes_dir,
        target,
        [this](const QString &absPath) -> QList<NoteItem> {
            QSet<QString> custom;
            return note_service_.parseNoteStructure(
                absPath, get_sections_from_ui(), custom, QFileInfo(absPath).baseName());
        },
        this);

    if (UiAnimator::execDialogSmooth(&dlg) != QDialog::Accepted)
        return;

    const QString source_file = dlg.sourceFilePath();
    const QString source_slug = dlg.sourceSlug();
    if (source_file.isEmpty() || source_slug.isEmpty()) {
        ui_.last_captured_label->setText(
            QStringLiteral("ইনজেক্ট: ফাইল বা উপ-শিরোনাম নির্বাচন হয়নি।"));
        return;
    }

    // Insert after currently selected heading in THIS file (empty = append at end)
    const QString insert_after = selected_heading_slug_;
    QString out;
    if (note_service_.injectSubheadingFromNote(target, insert_after, source_file, source_slug,
                                                   get_sections_from_ui(), out)) {
        // TOC already rebuilt inside injectSubheadingFromNote
        populate_headings_from_file();
        ui_.last_captured_label->setText(
            QStringLiteral("ইনজেক্ট ✓ %1 ← %2")
                .arg(dlg.sourceTitle(), QFileInfo(source_file).fileName()));
    } else {
        ui_.last_captured_label->setText(out.isEmpty()
            ? QStringLiteral("ইনজেক্ট ত্রুটি!")
            : out);
    }
}

void ClipboardGrabber::manual_append_to_heading() {
    // রিজিউম: always insert under the selected heading/subheading (exact hierarchy slot)
    if (ui_.subject_dropdown->currentIndex() == -1 || selected_heading_slug_.isEmpty()) {
        ui_.last_captured_label->setText(
            QStringLiteral("রিজিউম: আগে একটি শিরোনাম/উপ-শিরোনাম নির্বাচন করুন।"));
        return;
    }

    QClipboard::Mode mode = (ui_.mode_dropdown->currentIndex() == 0)
        ? QClipboard::Clipboard : QClipboard::Selection;
    QString text = QGuiApplication::clipboard()->text(mode).simplified();
    if (text.isEmpty()) {
        ui_.last_captured_label->setText(
            QStringLiteral("রিজিউম: ক্লিপবোর্ডে কোনো লেখা নেই।"));
        return;
    }

    QString target = get_current_target_file();
    // format from dropdown — bullet/paragraph/etc. goes UNDER the selected subheading
    if (note_service_.appendContentToHeading(target, selected_heading_slug_, text,
            ui_.format_dropdown->currentIndex(),
            ui_.section_dropdown->currentData().toString())) {
        note_service_.updateTocInFile(target, get_sections_from_ui());
        ui_.last_captured_label->setText(
            QStringLiteral("রিজিউম ✓ [%1]: %2").arg(selected_heading_title_, text));
    } else {
        ui_.last_captured_label->setText(
            QStringLiteral("রিজিউম ত্রুটি: নির্বাচিত স্থানে লেখা যায়নি!"));
    }
}

void ClipboardGrabber::shift_selected_heading_section() {
    if (ui_.subject_dropdown->currentIndex() == -1)
        return;

    QString target_file = get_current_target_file();
    if (target_file == QStringLiteral("নির্বাচিত নয়") || target_file.isEmpty())
        return;

    populate_headings_from_file();
    if (all_headings_.isEmpty()) {
        ui_.last_captured_label->setText(
            QStringLiteral("স্থানান্তর: কোনো শিরোনাম পাওয়া যায়নি।"));
        return;
    }

    // Drag-and-drop reorder dialog (preserves slugs → TOC links stay valid)
    HeadingReorderDialog dlg(all_headings_, selected_heading_slug_, this);
    connect(&dlg, &HeadingReorderDialog::moveRequested, this,
            [this, &dlg, target_file](const QString &sourceSlug, const QString &targetSlug) {
                if (sourceSlug.isEmpty())
                    return;
                QString out;
                if (note_service_.shiftHeadingSection(target_file, sourceSlug, targetSlug,
                                                      all_headings_, out)) {
                    note_service_.updateTocInFile(target_file, get_sections_from_ui());
                    populate_headings_from_file();
                    dlg.reload(all_headings_, sourceSlug);
                    selected_heading_slug_ = sourceSlug;
                    // refresh title from new list
                    for (const NoteItem &it : all_headings_) {
                        if (it.slug == sourceSlug) {
                            selected_heading_title_ = it.title;
                            ui_.select_heading_button->setText(it.title);
                            break;
                        }
                    }
                }
                ui_.last_captured_label->setText(out);
            });
    UiAnimator::execDialogSmooth(&dlg);
    update_button_states();
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

