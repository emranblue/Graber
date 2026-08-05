#include "ClipboardGrabber.h"
#include "MarkdownUtils.h"
#include "utils/Utils.h"

#include <QFile>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QLineEdit>
#include <QIcon>

void ClipboardGrabber::toggle_subject() {
    if (ui_.subject_dropdown->isEnabled() && ui_.subject_dropdown->count() > 0) {
        int next = (ui_.subject_dropdown->currentIndex() + 1) % ui_.subject_dropdown->count();
        ui_.subject_dropdown->setCurrentIndex(next);
    }
}

void ClipboardGrabber::add_subject() {
    QString selected_folder = ui_.folder_dropdown->currentData().toString();
    QString default_prefix;
    if (!selected_folder.isEmpty() && selected_folder != "__ALL__" && selected_folder != "__ROOT__")
        default_prefix = selected_folder + "/";

    bool ok = false;
    QString text = QInputDialog::getText(this, "বিষয় যোগ করুন",
        "নতুন বিষয়ের নাম (ফোল্ডার সহ, যেমন: BCS/Bangla):",
        QLineEdit::Normal, default_prefix, &ok);
    if (!ok || text.isEmpty()) return;

    text = text.trimmed().replace('\\', '/');
    note_service_.createSubject(text);

    int last_slash = text.lastIndexOf('/');
    if (last_slash != -1) {
        populate_folders_from_disk();
        int f_idx = ui_.folder_dropdown->findData(text.left(last_slash));
        if (f_idx != -1) {
            ui_.folder_dropdown->blockSignals(true);
            ui_.folder_dropdown->setCurrentIndex(f_idx);
            ui_.folder_dropdown->blockSignals(false);
        }
    }

    populate_subjects_from_disk();
    int s_idx = ui_.subject_dropdown->findData(text);
    if (s_idx != -1)
        ui_.subject_dropdown->setCurrentIndex(s_idx);
}

void ClipboardGrabber::add_folder() {
    bool ok = false;
    QString text = QInputDialog::getText(this, "ফোল্ডার তৈরি করুন",
        "নতুন ফোল্ডারের নাম (Path):", QLineEdit::Normal, "", &ok);
    if (!ok || text.isEmpty()) return;

    text = text.trimmed().replace('\\', '/');
    QString status_msg;
    if (note_service_.createFolder(text, status_msg)) {
        populate_folders_from_disk();
        int idx = ui_.folder_dropdown->findData(text);
        if (idx != -1)
            ui_.folder_dropdown->setCurrentIndex(idx);
        else
            populate_subjects_from_disk();
    }
    ui_.status_label->setText(status_msg);
}

void ClipboardGrabber::on_folder_changed(int) {
    populate_subjects_from_disk();
}

void ClipboardGrabber::on_subject_changed(const QString &/*text*/) {
    // Always resolve the subject via itemData (fullPath, e.g. "BCS/Bangla"),
    // never the display label ("📄 BCS / Bangla"). Using the display text made
    // loadSectionsForSubject look for a non-existent ".ini" and left the
    // section dropdown permanently blank.
    selected_heading_slug_.clear();
    selected_heading_title_.clear();
    ui_.select_heading_button->setText("(শেষে নতুন করে যোগ করুন / Append to End)");
    const QString subject = get_selected_subject_name();
    load_sections_for_subject(subject);
    populate_headings_from_file();
}

void ClipboardGrabber::populate_folders_from_disk() {
    QString current = ui_.folder_dropdown->currentData().toString();
    ui_.folder_dropdown->blockSignals(true);
    ui_.folder_dropdown->clear();

    static const QIcon folder_icon = get_folder_icon();
    ui_.folder_dropdown->addItem(folder_icon, "সকল ফোল্ডার (All Folders)", "__ALL__");
    ui_.folder_dropdown->addItem(folder_icon, "রুট ফোল্ডার (Root / Base)", "__ROOT__");

    for (const QString &folder : note_service_.populateFolders())
        ui_.folder_dropdown->addItem(folder_icon, folder, folder);

    int idx = current.isEmpty() ? -1 : ui_.folder_dropdown->findData(current);
    ui_.folder_dropdown->setCurrentIndex(idx != -1 ? idx : 0);
    ui_.folder_dropdown->blockSignals(false);
}

QString ClipboardGrabber::get_selected_subject_name() const {
    if (ui_.subject_dropdown->currentIndex() == -1) return "";
    QString data = ui_.subject_dropdown->currentData().toString();
    return data.isEmpty() ? ui_.subject_dropdown->currentText() : data;
}

void ClipboardGrabber::populate_subjects_from_disk() {
    QString current = get_selected_subject_name();
    QString filter = ui_.folder_dropdown->currentData().toString();
    if (filter.isEmpty()) filter = "__ALL__";

    ui_.subject_dropdown->blockSignals(true);
    ui_.subject_dropdown->clear();

    static const QIcon file_icon = get_file_icon();
    QList<SubjectItem> items = note_service_.populateSubjectItems(get_sections_from_ui(), filter);
    int select_idx = -1;
    for (int i = 0; i < items.size(); ++i) {
        ui_.subject_dropdown->addItem(file_icon, items[i].displayName, items[i].fullPath);
        if (!current.isEmpty() && items[i].fullPath == current)
            select_idx = i;
    }

    if (select_idx != -1)
        ui_.subject_dropdown->setCurrentIndex(select_idx);
    else if (ui_.subject_dropdown->count() > 0)
        ui_.subject_dropdown->setCurrentIndex(0);
    else
        ui_.subject_dropdown->setCurrentIndex(-1);

    ui_.subject_dropdown->blockSignals(false);
    on_subject_changed(get_selected_subject_name());
}

QString ClipboardGrabber::get_current_target_file() {
    return note_service_.getTargetFilePath(get_selected_subject_name());
}

void ClipboardGrabber::open_selected_file() {
    QString path = get_current_target_file();
    if (path != "নির্বাচিত নয়" && QFile::exists(path))
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
