#ifndef SECTION_DEFINITIONS_H
#define SECTION_DEFINITIONS_H

#include <QString>
#include <QList>

struct SectionDef {
    QString key;
    QString bangla;
    QString english;
};

class SectionDefinitions {
public:
    static QList<SectionDef> getDefaultSections();
    static QString detectSectionFromTitle(const QString &title);
};

#endif // SECTION_DEFINITIONS_H
