#include "NoteRepository.h"
#include "MarkdownUtils.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>

QStringList NoteRepository::populateSubjectsFromDisk(const QList<SectionItem> &sections,
                                                     const QString &folderFilter) {
    QStringList all_subjects;
    QDirIterator it(notes_dir_path_, QStringList() << "*.md", QDir::Files, QDirIterator::Subdirectories);
    QDir base_dir(notes_dir_path_);
    while (it.hasNext()) {
        QString filepath = it.next();
        QString relative_path = base_dir.relativeFilePath(filepath);
        relative_path.replace('\\', '/');

        if (relative_path == "deleted" || relative_path.startsWith("deleted/")) {
            continue;
        }

        relative_path.chop(3); // Remove ".md"

        if (!folderFilter.isEmpty() && folderFilter != "__ALL__") {
            if (folderFilter == "__ROOT__") {
                if (relative_path.contains('/')) {
                    continue;
                }
            } else {
                QString norm_filter = folderFilter;
                norm_filter.replace('\\', '/');
                QString prefix = norm_filter + "/";
                if (!relative_path.startsWith(prefix)) {
                    continue;
                }
            }
        }

        // Do NOT call updateTocInFile here. `sections` is the *currently
        // selected* subject's section list (or empty during startup). Applying
        // it to every .md under GraberNotes wiped other subjects' TOC groups
        // and left them blank. TOC is refreshed only when that specific note
        // is written to / the user starts monitoring it.
        // normalizeNoteFile is also skipped on every list refresh — it is
        // expensive and mutates files the user is not actively editing.
        Q_UNUSED(sections);

        all_subjects << relative_path;
    }
    all_subjects.sort(Qt::CaseInsensitive);
    return all_subjects;
}

QList<SubjectItem> NoteRepository::populateSubjectItemsFromDisk(const QList<SectionItem> &sections,
                                                                const QString &folderFilter) {
    QList<SubjectItem> items;
    QStringList raw_subjects = populateSubjectsFromDisk(sections, folderFilter);
    for (const QString &full : raw_subjects) {
        SubjectItem item;
        item.fullPath = full;
        int last_slash = full.lastIndexOf('/');
        if (last_slash != -1) {
            item.folderPath = full.left(last_slash);
            if (!folderFilter.isEmpty() && folderFilter != "__ALL__" && folderFilter != "__ROOT__") {
                item.displayName = "📄 " + full.mid(last_slash + 1);
            } else {
                item.displayName = "📄 " + item.folderPath + " / " + full.mid(last_slash + 1);
            }
        } else {
            item.folderPath = "";
            if (!folderFilter.isEmpty() && folderFilter != "__ALL__" && folderFilter != "__ROOT__") {
                item.displayName = "📄 " + full;
            } else {
                item.displayName = "📄 " + full + " (Root)";
            }
        }
        items.append(item);
    }
    return items;
}

bool NoteRepository::createSubject(const QString &subjectName) {
    if (subjectName.isEmpty()) return false;
    QString filename = notes_dir_path_ + QDir::separator() + subjectName + ".md";
    QFileInfo file_info(filename);
    QDir parent_dir = file_info.dir();
    if (!parent_dir.exists()) {
        parent_dir.mkpath(".");
    }
    if (!QFile::exists(filename)) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            return true;
        }
    }
    return true;
}

bool NoteRepository::moveSubject(const QString &oldSubjectName, const QString &newSubjectName,
                                 QString &outStatusMsg) {
    if (oldSubjectName.isEmpty() || newSubjectName.isEmpty()) {
        outStatusMsg = "অকার্যকর বিষয়ের নাম!";
        return false;
    }

    QString old_md = notes_dir_path_ + QDir::separator() + oldSubjectName + ".md";
    QString new_md = notes_dir_path_ + QDir::separator() + newSubjectName + ".md";

    if (!QFile::exists(old_md)) {
        outStatusMsg = "মূল বিষয় ফাইলটি পাওয়া যায়নি!";
        return false;
    }

    QFileInfo new_info(new_md);
    QDir().mkpath(new_info.absolutePath());

    if (!QFile::rename(old_md, new_md)) {
        if (!QFile::copy(old_md, new_md) || !QFile::remove(old_md)) {
            outStatusMsg = "নোট ফাইল স্থানান্তর ব্যর্থ হয়েছে!";
            return false;
        }
    }

    // Move associated .ini section config
    QString old_ini = notes_dir_path_ + QDir::separator() + oldSubjectName + ".ini";
    QString new_ini = notes_dir_path_ + QDir::separator() + newSubjectName + ".ini";
    if (QFile::exists(old_ini)) {
        if (!QFile::rename(old_ini, new_ini)) {
            if (QFile::copy(old_ini, new_ini)) {
                QFile::remove(old_ini);
            }
        }
    }

    // Move associated .tree structure file
    QString old_tree = notes_dir_path_ + QDir::separator() + oldSubjectName + ".tree";
    QString new_tree = notes_dir_path_ + QDir::separator() + newSubjectName + ".tree";
    if (QFile::exists(old_tree)) {
        if (!QFile::rename(old_tree, new_tree)) {
            if (QFile::copy(old_tree, new_tree)) {
                QFile::remove(old_tree);
            }
        }
    }

    outStatusMsg = "বিষয় সফলভাবে স্থানান্তর করা হয়েছে!";
    return true;
}
