#include "ClipboardGrabber.h"
#include "MarkdownUtils.h"

#include <QFile>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QLineEdit>
#include <QIcon>

QList<SectionItem> ClipboardGrabber::get_sections_from_ui() const {
    QList<SectionItem> sections;
    for (int i = 0; i < ui_.section_dropdown->count(); ++i)
        sections.append({ui_.section_dropdown->itemText(i),
                         ui_.section_dropdown->itemData(i).toString()});
    return sections;
}

void ClipboardGrabber::populate_sections_ui(const QList<SectionItem> &sections) {
    ui_.section_dropdown->blockSignals(true);
    ui_.section_dropdown->clear();
    for (const auto &sec : sections)
        ui_.section_dropdown->addItem(sec.displayName, sec.slug);
    ui_.section_dropdown->blockSignals(false);
}

void ClipboardGrabber::save_sections_for_subject(const QString &subject_name) {
    // Persist exactly what's in the section dropdown. Nothing here is ever a
    // built-in default — every entry either came from this subject's own
    // .ini (loaded earlier) or was just added by the user via "Add Section",
    // so there is nothing to filter out before writing it back.
    note_service_.saveSectionsForSubject(subject_name, get_sections_from_ui());
}

void ClipboardGrabber::load_sections_for_subject(const QString &subject_name) {
    populate_sections_ui(note_service_.loadSectionsForSubject(subject_name));
}

void ClipboardGrabber::add_section() {
    bool ok = false;
    QString text = QInputDialog::getText(this, "নতুন বিভাগ যোগ করুন",
        "নতুন বিভাগের নাম (যেমন: আইন ও সংবিধান / Law-Constitution):",
        QLineEdit::Normal, "", &ok);
    if (!ok || text.isEmpty()) return;

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

