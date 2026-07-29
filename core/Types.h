#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QKeySequence>
#include <QShortcut>
#include <QList>

enum class CaptureFormat {
    BulletPoint = 0,
    MainHeading = 1,
    Subheading = 2
};

enum class CaptureMode {
    Clipboard = 0,
    Selection = 1
};

struct ShortcutConfig {
    QString action_id;
    QString name_bangla;
    QString name_english;
    QString default_key;
    QKeySequence current_key;
    QShortcut* shortcut_obj = nullptr;
};

struct NoteItem {
    QString title;
    QString slug;
    QString type; // "heading" or "subheading"
    QString section;
    QString parent_slug;
};

struct SectionItem {
    QString displayName;
    QString slug;

    bool operator==(const SectionItem &other) const {
        return slug == other.slug && displayName == other.displayName;
    }
};

struct SubjectItem {
    QString fullPath;    // e.g. "BCS/Bangla"
    QString displayName; // e.g. "Bangla" or "BCS / Bangla"
    QString folderPath;  // e.g. "BCS"

    bool operator==(const SubjectItem &other) const {
        return fullPath == other.fullPath;
    }
};

#endif // TYPES_H
