#ifndef ISECTIONREPOSITORY_H
#define ISECTIONREPOSITORY_H

#include <QString>
#include <QList>
#include "Types.h"

class ISectionRepository {
public:
    virtual ~ISectionRepository() = default;

    virtual QList<SectionItem> loadSectionsForSubject(const QString &notesDirPath, const QString &subjectName) = 0;
    virtual void saveSectionsForSubject(const QString &notesDirPath, const QString &subjectName, const QList<SectionItem> &sections) = 0;
};

#endif // ISECTIONREPOSITORY_H
