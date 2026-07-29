#ifndef INISECTIONREPOSITORY_H
#define INISECTIONREPOSITORY_H

#include "interfaces/ISectionRepository.h"

class IniSectionRepository : public ISectionRepository {
public:
    IniSectionRepository() = default;
    ~IniSectionRepository() override = default;

    QList<SectionItem> loadSectionsForSubject(const QString &notesDirPath, const QString &subjectName) override;
    void saveSectionsForSubject(const QString &notesDirPath, const QString &subjectName, const QList<SectionItem> &sections) override;
    QList<SectionItem> getDefaultSections() const override;
};

#endif // INISECTIONREPOSITORY_H
