#ifndef NOTEREPOSITORY_H
#define NOTEREPOSITORY_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QSet>
#include <QComboBox>
#include <QImage>
#include "Types.h"

class NoteRepository {
public:
    NoteRepository();
    
    QString notesDirPath() const;
    QStringList populateSubjectsFromDisk(QComboBox *sectionDropdown);
    
    bool createSubject(const QString &subjectName);
    bool createFolder(const QString &folderPath, QString &outStatusMsg);
    
    void saveSectionsForSubject(const QString &subjectName, QComboBox *sectionDropdown);
    void loadSectionsForSubject(const QString &subjectName, QComboBox *sectionDropdown);
    
    QString getTargetFilePath(const QString &subjectName) const;
    
    void normalizeMarkdownFile(const QString &filePath);
    void updateTocInFile(const QString &filePath, QComboBox *sectionDropdown);
    
    void parseNoteStructure(const QString &filePath, QList<NoteItem> &items, QComboBox *sectionDropdown, const QSet<QString> &customAddedSections, const QString &subjectName);
    
    bool appendContentToHeading(const QString &filePath, const QString &slug, const QString &processedText, int formatIndex = 0, const QString &section = "others");
    
    bool writeToNote(const QString &targetFile, const QString &processedText, int formatIndex, const QString &section, const QString &selectedSlug, QString &lastDate, QString &outCapturedLabelText);
    
    bool writeImageToNote(const QString &targetFile, const QString &imageFilename, QString &lastDate);
    
    bool injectHeadingToNote(const QString &targetFile, const QString &simplifiedText, const QString &section, QString &lastDate);
    
    bool deleteHeadingSection(const QString &targetFile, const QString &slug, const QString &subjectName, QString &outCapturedLabelText);
    
    bool shiftHeadingSection(const QString &targetFile, const QString &sourceSlug, const QString &targetSlug, const QList<NoteItem> &allHeadings, QString &outCapturedLabelText);

private:
    QString notes_dir_path_;
};

#endif // NOTEREPOSITORY_H
