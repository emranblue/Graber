#include "subject_manager.h"
#include "../../utilities/file_handler.h"
#include "../../utilities/logger.h"
#include <QDir>
#include <QFileInfo>

SubjectManager::SubjectManager(const QString &notesDir)
    : notes_dir_path_(notesDir) {
    FileHandler::ensureDirectoryExists(notesDir);
}

QStringList SubjectManager::getAllSubjects() const {
    return FileHandler::getAllMarkdownFiles(notes_dir_path_);
}

void SubjectManager::addSubject(const QString &subjectName) {
    QString filename = notes_dir_path_ + QDir::separator() + subjectName + ".md";
    QFileInfo file_info(filename);
    QDir parent_dir = file_info.dir();
    
    if (!parent_dir.exists()) {
        parent_dir.mkpath(".");
    }
    
    if (!FileHandler::fileExists(filename)) {
        FileHandler::writeFile(filename, "");
        Logger::info(QString("Subject created: %1").arg(subjectName));
    }
}

void SubjectManager::addFolder(const QString &folderName) {
    QDir dir(notes_dir_path_);
    if (dir.mkpath(folderName)) {
        Logger::info(QString("Folder created: %1").arg(folderName));
    }
}

QString SubjectManager::getCurrentSubjectFile(const QString &currentSubject) const {
    if (currentSubject.isEmpty()) {
        return "";
    }
    return notes_dir_path_ + QDir::separator() + currentSubject + ".md";
}

void SubjectManager::refreshSubjects() {
    // Refresh subject list from disk
}
