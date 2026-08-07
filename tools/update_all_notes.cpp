#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>
#include "../formatters/MarkdownDocumentFormatter.h"
#include "../repositories/NoteRepository.h"
#include "../repositories/IniSectionRepository.h"
#include "../utils/FileIO.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    MarkdownDocumentFormatter formatter;
    IniSectionRepository sectionRepo;
    NoteRepository noteRepo;

    QString rootDir = QDir::homePath() + QStringLiteral("/GraberNotes");
    qDebug() << "Scanning GraberNotes directory:" << rootDir;

    QDirIterator it(rootDir, QStringList() << QStringLiteral("*.md"), QDir::Files, QDirIterator::Subdirectories);

    int count = 0;
    while (it.hasNext()) {
        QString mdPath = it.next();
        QFileInfo info(mdPath);
        QString relPath = QDir(rootDir).relativeFilePath(mdPath);
        qDebug() << "--------------------------------------------------";
        qDebug() << "Processing Note #" << (++count) << ":" << relPath;

        // Subject name is relPath without .md extension (e.g. "Bangla/বাংলা_সাহিত্য")
        QString subjectName = relPath;
        if (subjectName.endsWith(QStringLiteral(".md"), Qt::CaseInsensitive))
            subjectName.chop(3);

        // 1. Load sections for this subject from its .ini file
        QList<SectionItem> sections = sectionRepo.loadSectionsForSubject(rootDir, subjectName);

        // 2. Update TOC in the .md file with glossy card design
        noteRepo.updateTocInFile(mdPath, sections);

        // 3. Parse note structure tree & update .tree file, collect any custom sections
        QSet<QString> customSections;
        QList<NoteItem> items = noteRepo.parseNoteStructure(mdPath, sections, customSections, subjectName);

        // 4. If custom sections exist in note content, update subject sections and write back .ini file
        if (!customSections.isEmpty()) {
            bool modified = false;
            for (const QString &customSlug : customSections) {
                bool exists = false;
                for (const auto &sec : sections) {
                    if (sec.slug == customSlug) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    QString disp = customSlug;
                    if (!disp.isEmpty()) disp[0] = disp[0].toUpper();
                    sections.append(SectionItem{disp, customSlug});
                    modified = true;
                }
            }
            if (modified) {
                sectionRepo.saveSectionsForSubject(rootDir, subjectName, sections);
                // Re-update TOC so the new section titles are properly aligned
                noteRepo.updateTocInFile(mdPath, sections);
                qDebug() << "Updated .ini for" << subjectName << "with custom sections.";
            }
        }
        qDebug() << "Successfully updated:" << mdPath;
        qDebug() << "Items written to .tree file:" << items.size();
    }

    qDebug() << "==================================================";
    qDebug() << "Total notes processed:" << count;
    qDebug() << "All GraberNotes files (.md, .ini, .tree) successfully updated!";
    return 0;
}
