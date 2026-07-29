#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QKeySequence>
#include <QShortcut>

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
};

#endif // TYPES_H
