#ifndef SECTION_MANAGER_H
#define SECTION_MANAGER_H

#include <QString>
#include <QStringList>
#include <QComboBox>

class SectionManager {
public:
    explicit SectionManager(const QString &notesDir);
    
    void loadSectionsForSubject(const QString &subject, QComboBox *dropdown);
    void saveSectionsForSubject(const QString &subject, QComboBox *dropdown);
    void addCustomSection(const QString &sectionName, QComboBox *dropdown);
    
private:
    QString notes_dir_path_;
    QString getSectionsIniPath(const QString &subject) const;
};

#endif // SECTION_MANAGER_H
