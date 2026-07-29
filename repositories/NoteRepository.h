#ifndef NOTEREPOSITORY_H
#define NOTEREPOSITORY_H

#include "interfaces/INoteRepository.h"
#include "interfaces/IDocumentFormatter.h"
#include "interfaces/ISectionRepository.h"
#include <memory>

class NoteRepository : public INoteRepository {
public:
    NoteRepository();
    explicit NoteRepository(std::shared_ptr<IDocumentFormatter> formatter,
                           std::shared_ptr<ISectionRepository> sectionRepo);
    ~NoteRepository() override = default;

    QString notesDirPath() const override;
    QStringList populateSubjectsFromDisk(const QList<SectionItem> &sections) override;

    bool createSubject(const QString &subjectName) override;
    bool createFolder(const QString &folderPath, QString &outStatusMsg) override;

    QList<SectionItem> loadSectionsForSubject(const QString &subjectName) override;
    void saveSectionsForSubject(const QString &subjectName, const QList<SectionItem> &sections) override;

    QString getTargetFilePath(const QString &subjectName) const override;

    void normalizeNoteFile(const QString &filePath) override;
    void updateTocInFile(const QString &filePath, const QList<SectionItem> &sections) override;

    QList<NoteItem> parseNoteStructure(const QString &filePath, const QList<SectionItem> &sections, QSet<QString> &customAddedSections, const QString &subjectName) override;

    bool appendContentToHeading(const QString &filePath, const QString &slug, const QString &processedText, int formatIndex = 0, const QString &section = "others") override;

    bool writeToNote(const QString &targetFile, const QString &processedText, int formatIndex, const QString &section, const QString &selectedSlug, QString &lastDate, QString &outCapturedLabelText) override;

    bool writeImageToNote(const QString &targetFile, const QString &imageFilename, QString &lastDate) override;

    bool injectHeadingToNote(const QString &targetFile, const QString &simplifiedText, const QString &section, QString &lastDate) override;

    bool deleteHeadingSection(const QString &targetFile, const QString &slug, const QString &subjectName, QString &outCapturedLabelText) override;

    bool shiftHeadingSection(const QString &targetFile, const QString &sourceSlug, const QString &targetSlug, const QList<NoteItem> &allHeadings, QString &outCapturedLabelText) override;

private:
    QString notes_dir_path_;
    std::shared_ptr<IDocumentFormatter> formatter_;
    std::shared_ptr<ISectionRepository> section_repo_;
};

#endif // NOTEREPOSITORY_H
