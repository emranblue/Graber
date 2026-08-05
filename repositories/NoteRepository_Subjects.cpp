#include "NoteRepository.h"
#include "MarkdownUtils.h"
#include "../utils/Utils.h"
#include "../utils/FileIO.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>

namespace {

/** Rename-or-copy-then-remove a single sidecar file. Best-effort; never throws. */
void moveSidecar(const QString &oldPath, const QString &newPath) {
    if (!QFile::exists(oldPath))
        return;
    QFileInfo newInfo(newPath);
    QDir().mkpath(newInfo.absolutePath());
    if (QFile::rename(oldPath, newPath))
        return;
    if (QFile::copy(oldPath, newPath))
        QFile::remove(oldPath);
}

} // namespace

QStringList NoteRepository::populateSubjectsFromDisk(const QList<SectionItem> &sections,
                                                     const QString &folderFilter) {
    QStringList all_subjects;
    QDirIterator it(notes_dir_path_, QStringList() << "*.md", QDir::Files, QDirIterator::Subdirectories);
    QDir base_dir(notes_dir_path_);
    while (it.hasNext()) {
        QString filepath = it.next();
        QString relative_path = base_dir.relativeFilePath(filepath);
        relative_path.replace('\\', '/');

        if (relative_path == "deleted" || relative_path.startsWith("deleted/") ||
            relative_path == "config" || relative_path.startsWith("config/")) {
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
    const QString safe = sanitizeRelativePath(subjectName);
    if (safe.isEmpty()) return false;
    QString filename = notes_dir_path_ + QDir::separator() + safe + ".md";
    QFileInfo file_info(filename);
    QDir parent_dir = file_info.dir();
    if (!parent_dir.exists()) {
        parent_dir.mkpath(".");
    }
    if (!QFile::exists(filename)) {
        // Atomic empty create via FileIO when possible.
        if (FileIO::writeTextAtomic(filename, QString()).isOk())
            return true;
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
            return true;
        }
        return false;
    }
    return true;
}

bool NoteRepository::moveSubject(const QString &oldSubjectName, const QString &newSubjectName,
                                 QString &outStatusMsg) {
    // Path-traversal hard stop: both names must sanitize to a safe relative path
    // under notes_dir_path_. Never concatenate raw user strings into filesystem paths.
    const QString oldSafe = sanitizeRelativePath(oldSubjectName);
    const QString newSafe = sanitizeRelativePath(newSubjectName);

    if (oldSafe.isEmpty() || newSafe.isEmpty()) {
        outStatusMsg = QStringLiteral("অকার্যকর বিষয়ের নাম!");
        return false;
    }
    if (oldSafe == newSafe) {
        outStatusMsg = QStringLiteral("পুরনো ও নতুন নাম একই!");
        return false;
    }

    const QString old_md = notes_dir_path_ + QDir::separator() + oldSafe + QStringLiteral(".md");
    const QString new_md = notes_dir_path_ + QDir::separator() + newSafe + QStringLiteral(".md");

    if (!QFile::exists(old_md)) {
        outStatusMsg = QStringLiteral("মূল বিষয় ফাইলটি পাওয়া যায়নি!");
        return false;
    }
    if (QFile::exists(new_md)) {
        outStatusMsg = QStringLiteral("গন্তব্য বিষয় ইতিমধ্যে বিদ্যমান!");
        return false;
    }

    QFileInfo new_info(new_md);
    if (!QDir().mkpath(new_info.absolutePath())) {
        outStatusMsg = QStringLiteral("গন্তব্য ফোল্ডার তৈরি করা যায়নি!");
        return false;
    }

    if (!QFile::rename(old_md, new_md)) {
        if (!QFile::copy(old_md, new_md) || !QFile::remove(old_md)) {
            outStatusMsg = QStringLiteral("নোট ফাইল স্থানান্তর ব্যর্থ হয়েছে!");
            return false;
        }
    }

    // Sidecars: .ini section config + .tree outline (best-effort)
    moveSidecar(notes_dir_path_ + QDir::separator() + oldSafe + QStringLiteral(".ini"),
                notes_dir_path_ + QDir::separator() + newSafe + QStringLiteral(".ini"));
    moveSidecar(notes_dir_path_ + QDir::separator() + oldSafe + QStringLiteral(".tree"),
                notes_dir_path_ + QDir::separator() + newSafe + QStringLiteral(".tree"));

    outStatusMsg = QStringLiteral("বিষয় সফলভাবে স্থানান্তর করা হয়েছে!");
    return true;
}
