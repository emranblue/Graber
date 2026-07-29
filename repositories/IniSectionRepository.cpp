#include "IniSectionRepository.h"
#include "MarkdownUtils.h"
#include <QFileInfo>
#include <QDir>
#include <QSettings>

QList<SectionItem> IniSectionRepository::getDefaultSections() const {
    return MarkdownUtils::get_default_sections();
}

QList<SectionItem> IniSectionRepository::loadSectionsForSubject(const QString &notesDirPath, const QString &subjectName) {
    if (subjectName.isEmpty() || subjectName == "নির্বাচিত নয়") {
        return getDefaultSections();
    }

    QString ini_path = notesDirPath + QDir::separator() + subjectName + ".ini";
    if (!QFile::exists(ini_path)) {
        return getDefaultSections();
    }

    QSettings settings(ini_path, QSettings::IniFormat);
    settings.beginGroup("Sections");
    QString order_str = settings.value("_order", "").toString();
    
    QList<SectionItem> sections;
    if (!order_str.isEmpty()) {
        QStringList order = order_str.split(",");
        for (const QString &slug : order) {
            if (settings.contains(slug)) {
                QString display_name = settings.value(slug).toString();
                sections.append({display_name, slug});
            }
        }
    } else {
        QStringList keys = settings.allKeys();
        for (const QString &key : keys) {
            if (key != "_order") {
                QString display_name = settings.value(key).toString();
                sections.append({display_name, key});
            }
        }
    }
    settings.endGroup();

    if (sections.isEmpty()) {
        sections = getDefaultSections();
    }
    return sections;
}

void IniSectionRepository::saveSectionsForSubject(const QString &notesDirPath, const QString &subjectName, const QList<SectionItem> &sections) {
    if (subjectName.isEmpty() || subjectName == "নির্বাচিত নয়") return;

    QString ini_path = notesDirPath + QDir::separator() + subjectName + ".ini";
    QFileInfo fileInfo(ini_path);
    QDir().mkpath(fileInfo.absolutePath());

    QSettings settings(ini_path, QSettings::IniFormat);
    settings.beginGroup("Sections");
    settings.remove("");

    QStringList order;
    for (const SectionItem &sec : sections) {
        order.append(sec.slug);
        settings.setValue(sec.slug, sec.displayName);
    }
    settings.setValue("_order", order.join(","));
    settings.endGroup();
}
