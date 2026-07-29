#ifndef NOTESERVICE_H
#define NOTESERVICE_H

#include "interfaces/INoteRepository.h"
#include <memory>
#include <QObject>
#include <QList>
#include <QStringList>

class NoteService : public QObject {
    Q_OBJECT
public:
    explicit NoteService(QObject *parent = nullptr);
    explicit NoteService(std::shared_ptr<INoteRepository> noteRepo, QObject *parent = nullptr);
    ~NoteService() override = default;

    QString notesDirPath() const;
    QStringList populateSubjects(const QList<SectionItem> &sections);
    bool createSubject(const QString &subjectName);
    bool createFolder(const QString &folderPath, QString &outStatusMsg);

    QList<SectionItem> loadSectionsForSubject(const QString &subjectName);
    void saveSectionsForSubject(const QString &subjectName, const QList<SectionItem> &sections);

    QString getTargetFilePath(const QString &subjectName) const;
    void normalizeNoteFile(const QString &filePath);
    void updateTocInFile(const QString &filePath, const QList<SectionItem> &sections);

    QList<NoteItem> parseNoteStructure(const QString &filePath, const QList<SectionItem> &sections, QSet<QString> &customAddedSections, const QString &subjectName);

    bool appendContentToHeading(const QString &filePath, const QString &slug, const QString &processedText, int formatIndex, const QString &section);
    bool writeToNote(const QString &targetFile, const QString &processedText, int formatIndex, const QString &section, const QString &selectedSlug, QString &lastDate, QString &outCapturedLabelText);
    bool writeImageToNote(const QString &targetFile, const QString &imageFilename, QString &lastDate);
    bool injectHeadingToNote(const QString &targetFile, const QString &simplifiedText, const QString &section, QString &lastDate);
    bool deleteHeadingSection(const QString &targetFile, const QString &slug, const QString &subjectName, QString &outCapturedLabelText);
    bool shiftHeadingSection(const QString &targetFile, const QString &sourceSlug, const QString &targetSlug, const QList<NoteItem> &allHeadings, QString &outCapturedLabelText);

private:
    std::shared_ptr<INoteRepository> note_repository_;
};

#endif // NOTESERVICE_H
