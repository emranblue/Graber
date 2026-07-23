#ifndef SUBJECT_MANAGER_H
#define SUBJECT_MANAGER_H

#include <QString>
#include <QStringList>

class SubjectManager {
public:
    explicit SubjectManager(const QString &notesDir);
    
    QStringList getAllSubjects() const;
    void addSubject(const QString &subjectName);
    void addFolder(const QString &folderName);
    QString getCurrentSubjectFile(const QString &currentSubject) const;
    
private:
    QString notes_dir_path_;
    
    void refreshSubjects();
};

#endif // SUBJECT_MANAGER_H
