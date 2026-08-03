#ifndef INOTEREPOSITORY_H
#define INOTEREPOSITORY_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QSet>
#include "Types.h"

class INoteRepository {
public:
    virtual ~INoteRepository() = default;

    virtual QString notesDirPath() const = 0;
    virtual QStringList populateFoldersFromDisk() = 0;
    virtual QStringList populateSubjectsFromDisk(const QList<SectionItem> &sections, const QString &folderFilter = "") = 0;
    virtual QList<SubjectItem> populateSubjectItemsFromDisk(const QList<SectionItem> &sections, const QString &folderFilter = "") = 0;

    virtual bool createSubject(const QString &subjectName) = 0;
    virtual bool createFolder(const QString &folderPath, QString &outStatusMsg) = 0;
    virtual bool moveSubject(const QString &oldSubjectName, const QString &newSubjectName, QString &outStatusMsg) = 0;

    virtual QList<SectionItem> loadSectionsForSubject(const QString &subjectName) = 0;
    virtual void saveSectionsForSubject(const QString &subjectName, const QList<SectionItem> &sections) = 0;

    virtual QString getTargetFilePath(const QString &subjectName) const = 0;

    virtual void normalizeNoteFile(const QString &filePath) = 0;
    virtual void updateTocInFile(const QString &filePath, const QList<SectionItem> &sections) = 0;

    virtual QList<NoteItem> parseNoteStructure(const QString &filePath, const QList<SectionItem> &sections, QSet<QString> &customAddedSections, const QString &subjectName) = 0;

    virtual bool appendContentToHeading(const QString &filePath, const QString &slug, const QString &processedText, int formatIndex, const QString &section) = 0;

    virtual bool writeToNote(const QString &targetFile, const QString &processedText, int formatIndex, const QString &section, const QString &selectedSlug, QString &lastDate, QString &outCapturedLabelText) = 0;

    virtual bool writeImageToNote(const QString &targetFile, const QString &imageFilename, QString &lastDate) = 0;

    // Inserts a raw, already-formatted Markdown block (e.g. a ```mermaid
    // fenced diagram) as-is — unlike appendContentToHeading/writeToNote, the
    // text is not wrapped in any bullet/heading/paragraph markup. Goes into
    // the selected heading if selectedSlug is non-empty, otherwise it's
    // appended at the end of the file under the active date section.
    virtual bool insertDiagramToNote(const QString &targetFile, const QString &diagramMarkdown, const QString &selectedSlug, QString &lastDate) = 0;

    // Live/growing diagram: identified by sessionId. First call for a given
    // sessionId inserts a fresh marker-wrapped block (same placement rules
    // as insertDiagramToNote); subsequent calls with the same sessionId
    // replace that block in place, so a diagram can keep growing (new nodes
    // captured from the clipboard) without leaving stale duplicate copies
    // in the note.
    virtual bool upsertLiveDiagram(const QString &targetFile, const QString &sessionId, const QString &diagramMarkdown, const QString &selectedSlug, QString &lastDate) = 0;

    virtual bool injectHeadingToNote(const QString &targetFile, const QString &simplifiedText, const QString &section, QString &lastDate) = 0;

    virtual bool deleteHeadingSection(const QString &targetFile, const QString &slug, const QString &subjectName, QString &outCapturedLabelText) = 0;

    virtual bool shiftHeadingSection(const QString &targetFile, const QString &sourceSlug, const QString &targetSlug, const QList<NoteItem> &allHeadings, QString &outCapturedLabelText) = 0;
};

#endif // INOTEREPOSITORY_H
