#include "NoteRepository.h"
#include "MarkdownDocumentFormatter.h"
#include "IniSectionRepository.h"
#include "../utils/Utils.h"
#include "MarkdownUtils.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>

NoteRepository::NoteRepository()
    : formatter_(std::make_shared<MarkdownDocumentFormatter>()),
      section_repo_(std::make_shared<IniSectionRepository>()) {
    notes_dir_path_ = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                      + QDir::separator() + "GraberNotes";
    QDir dir(notes_dir_path_);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

NoteRepository::NoteRepository(std::shared_ptr<IDocumentFormatter> formatter,
                               std::shared_ptr<ISectionRepository> sectionRepo)
    : formatter_(std::move(formatter)),
      section_repo_(std::move(sectionRepo)) {
    notes_dir_path_ = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                      + QDir::separator() + "GraberNotes";
    QDir dir(notes_dir_path_);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString NoteRepository::notesDirPath() const {
    return notes_dir_path_;
}

QStringList NoteRepository::populateFoldersFromDisk() {
    QStringList folders;
    QDir base_dir(notes_dir_path_);
    QDirIterator it(notes_dir_path_, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString dir_path = it.next();
        QString rel = base_dir.relativeFilePath(dir_path);
        rel.replace('\\', '/');

        if (rel == "deleted" || rel.startsWith("deleted/") ||
            rel == "images" || rel.endsWith("/images") || rel.contains("/images/") ||
            rel == "backup" || rel.startsWith("backup/") ||
            rel.startsWith(".git") || rel.startsWith("build")) {
            continue;
        }
        folders << rel;
    }
    folders.sort(Qt::CaseInsensitive);
    return folders;
}

bool NoteRepository::createFolder(const QString &folderPath, QString &outStatusMsg) {
    if (folderPath.isEmpty()) return false;
    QDir dir(notes_dir_path_);
    if (dir.mkpath(folderPath)) {
        outStatusMsg = "অবস্থা: ফোল্ডার তৈরি হয়েছে - " + folderPath;
        return true;
    }
    outStatusMsg = "অবস্থা: ফোল্ডার তৈরি করতে ব্যর্থ!";
    return false;
}

QList<SectionItem> NoteRepository::loadSectionsForSubject(const QString &subjectName) {
    return section_repo_->loadSectionsForSubject(notes_dir_path_, subjectName);
}

void NoteRepository::saveSectionsForSubject(const QString &subjectName, const QList<SectionItem> &sections) {
    section_repo_->saveSectionsForSubject(notes_dir_path_, subjectName, sections);
}

QString NoteRepository::getTargetFilePath(const QString &subjectName) const {
    if (subjectName.isEmpty() || subjectName == "নির্বাচিত নয়") {
        return "নির্বাচিত নয়";
    }
    return notes_dir_path_ + QDir::separator() + subjectName + ".md";
}
