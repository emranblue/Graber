#ifndef NOTESERVICE_H
#define NOTESERVICE_H

#include "interfaces/INoteService.h"
#include "interfaces/INoteRepository.h"
#include <memory>
#include <QObject>
#include <QList>
#include <QStringList>

class NoteService : public QObject, public INoteService {
    Q_OBJECT
public:
    explicit NoteService(QObject *parent = nullptr);
    explicit NoteService(std::shared_ptr<INoteRepository> noteRepo, QObject *parent = nullptr);
    ~NoteService() override = default;

    QString notesDirPath() const override;
    QStringList populateFolders() override;
    QStringList populateSubjects(const QList<SectionItem> &sections, const QString &folderFilter = "") override;
    QList<SubjectItem> populateSubjectItems(const QList<SectionItem> &sections, const QString &folderFilter = "") override;
    bool createSubject(const QString &subjectName) override;
    bool createFolder(const QString &folderPath, QString &outStatusMsg) override;
    bool moveSubject(const QString &oldSubjectName, const QString &newSubjectName, QString &outStatusMsg) override;

    QList<SectionItem> loadSectionsForSubject(const QString &subjectName) override;
    void saveSectionsForSubject(const QString &subjectName, const QList<SectionItem> &sections) override;

    QString getTargetFilePath(const QString &subjectName) const override;
    void normalizeNoteFile(const QString &filePath) override;
    void updateTocInFile(const QString &filePath, const QList<SectionItem> &sections) override;

    QList<NoteItem> parseNoteStructure(const QString &filePath, const QList<SectionItem> &sections, QSet<QString> &customAddedSections, const QString &subjectName) override;

    bool appendContentToHeading(const QString &filePath, const QString &slug, const QString &processedText, int formatIndex, const QString &section) override;
    bool writeToNote(const QString &targetFile, const QString &processedText, int formatIndex, const QString &section, const QString &selectedSlug, QString &lastDate, QString &outCapturedLabelText) override;
    bool writeImageToNote(const QString &targetFile, const QString &imageFilename, const QString &selectedSlug, QString &lastDate) override;
    bool insertDiagramToNote(const QString &targetFile, const QString &diagramMarkdown, const QString &selectedSlug, QString &lastDate) override;
    bool upsertLiveDiagram(const QString &targetFile, const QString &sessionId, const QString &diagramMarkdown, const QString &selectedSlug, QString &lastDate) override;
    bool injectHeadingToNote(const QString &targetFile, const QString &simplifiedText, const QString &section, QString &lastDate) override;
    bool injectSubheadingFromNote(const QString &targetFile, const QString &insertAfterSlug,
                                  const QString &sourceFile, const QString &sourceSlug,
                                  const QList<SectionItem> &sections,
                                  QString &outCapturedLabelText) override;
    bool deleteHeadingSection(const QString &targetFile, const QString &slug, const QString &subjectName, QString &outCapturedLabelText) override;
    bool shiftHeadingSection(const QString &targetFile, const QString &sourceSlug, const QString &targetSlug, const QList<NoteItem> &allHeadings, QString &outCapturedLabelText) override;

private:
    std::shared_ptr<INoteRepository> note_repository_;
};

#endif // NOTESERVICE_H
