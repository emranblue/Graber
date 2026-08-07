#include "NoteService.h"
#include "NoteRepository.h"

NoteService::NoteService(QObject *parent)
    : QObject(parent), note_repository_(std::make_shared<NoteRepository>()) {}

NoteService::NoteService(std::shared_ptr<INoteRepository> noteRepo, QObject *parent)
    : QObject(parent), note_repository_(std::move(noteRepo)) {}

QString NoteService::notesDirPath() const {
    return note_repository_->notesDirPath();
}

QStringList NoteService::populateFolders() {
    return note_repository_->populateFoldersFromDisk();
}

QStringList NoteService::populateSubjects(const QList<SectionItem> &sections, const QString &folderFilter) {
    return note_repository_->populateSubjectsFromDisk(sections, folderFilter);
}

QList<SubjectItem> NoteService::populateSubjectItems(const QList<SectionItem> &sections, const QString &folderFilter) {
    return note_repository_->populateSubjectItemsFromDisk(sections, folderFilter);
}

bool NoteService::createSubject(const QString &subjectName) {
    return note_repository_->createSubject(subjectName);
}

bool NoteService::createFolder(const QString &folderPath, QString &outStatusMsg) {
    return note_repository_->createFolder(folderPath, outStatusMsg);
}

bool NoteService::moveSubject(const QString &oldSubjectName, const QString &newSubjectName, QString &outStatusMsg) {
    return note_repository_->moveSubject(oldSubjectName, newSubjectName, outStatusMsg);
}

QList<SectionItem> NoteService::loadSectionsForSubject(const QString &subjectName) {
    return note_repository_->loadSectionsForSubject(subjectName);
}

void NoteService::saveSectionsForSubject(const QString &subjectName, const QList<SectionItem> &sections) {
    note_repository_->saveSectionsForSubject(subjectName, sections);
}

QString NoteService::getTargetFilePath(const QString &subjectName) const {
    return note_repository_->getTargetFilePath(subjectName);
}

void NoteService::normalizeNoteFile(const QString &filePath) {
    note_repository_->normalizeNoteFile(filePath);
}

void NoteService::updateTocInFile(const QString &filePath, const QList<SectionItem> &sections) {
    note_repository_->updateTocInFile(filePath, sections);
}

QList<NoteItem> NoteService::parseNoteStructure(const QString &filePath, const QList<SectionItem> &sections, QSet<QString> &customAddedSections, const QString &subjectName) {
    return note_repository_->parseNoteStructure(filePath, sections, customAddedSections, subjectName);
}

bool NoteService::appendContentToHeading(const QString &filePath, const QString &slug, const QString &processedText, int formatIndex, const QString &section) {
    return note_repository_->appendContentToHeading(filePath, slug, processedText, formatIndex, section);
}

bool NoteService::writeToNote(const QString &targetFile, const QString &processedText, int formatIndex, const QString &section, const QString &selectedSlug, QString &lastDate, QString &outCapturedLabelText) {
    return note_repository_->writeToNote(targetFile, processedText, formatIndex, section, selectedSlug, lastDate, outCapturedLabelText);
}

bool NoteService::writeImageToNote(const QString &targetFile, const QString &imageFilename, const QString &selectedSlug, QString &lastDate) {
    return note_repository_->writeImageToNote(targetFile, imageFilename, selectedSlug, lastDate);
}

bool NoteService::insertDiagramToNote(const QString &targetFile, const QString &diagramMarkdown, const QString &selectedSlug, QString &lastDate) {
    return note_repository_->insertDiagramToNote(targetFile, diagramMarkdown, selectedSlug, lastDate);
}

bool NoteService::upsertLiveDiagram(const QString &targetFile, const QString &sessionId, const QString &diagramMarkdown, const QString &selectedSlug, QString &lastDate) {
    return note_repository_->upsertLiveDiagram(targetFile, sessionId, diagramMarkdown, selectedSlug, lastDate);
}

bool NoteService::injectHeadingToNote(const QString &targetFile, const QString &simplifiedText, const QString &section, QString &lastDate) {
    return note_repository_->injectHeadingToNote(targetFile, simplifiedText, section, lastDate);
}

bool NoteService::injectSubheadingFromNote(const QString &targetFile, const QString &insertAfterSlug,
                                           const QString &sourceFile, const QString &sourceSlug,
                                           const QList<SectionItem> &sections,
                                           QString &outCapturedLabelText) {
    return note_repository_->injectSubheadingFromNote(targetFile, insertAfterSlug, sourceFile, sourceSlug,
                                                      sections, outCapturedLabelText);
}

bool NoteService::deleteHeadingSection(const QString &targetFile, const QString &slug, const QString &subjectName, QString &outCapturedLabelText) {
    return note_repository_->deleteHeadingSection(targetFile, slug, subjectName, outCapturedLabelText);
}

bool NoteService::shiftHeadingSection(const QString &targetFile, const QString &sourceSlug, const QString &targetSlug, const QList<NoteItem> &allHeadings, QString &outCapturedLabelText) {
    return note_repository_->shiftHeadingSection(targetFile, sourceSlug, targetSlug, allHeadings, outCapturedLabelText);
}
