#include "IniSectionRepository.h"
#include "MarkdownUtils.h"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QSettings>

QList<SectionItem> IniSectionRepository::getDefaultSections() const {
    return MarkdownUtils::get_default_sections();
}

QList<SectionItem> IniSectionRepository::loadSectionsForSubject(const QString &notesDirPath, const QString &subjectName) {
    if (subjectName.isEmpty() || subjectName == "নির্বাচিত নয়") {
        return getDefaultSections();
    }

    QString ini_path = notesDirPath + QDir::separator() + subjectName + ".ini";
    
    // Auto-migration check: If target .ini does not exist, search notesDirPath for matching <basename>.ini
    if (!QFile::exists(ini_path)) {
        QFileInfo subject_info(subjectName);
        QString filename_stem = subject_info.fileName(); // e.g. "Bangla"
        if (!filename_stem.isEmpty()) {
            QString ini_filename = filename_stem + ".ini";

            QDirIterator it(notesDirPath, QStringList() << ini_filename, QDir::Files, QDirIterator::Subdirectories);
            if (it.hasNext()) {
                QString old_ini_path = it.next();
                if (old_ini_path != ini_path) {
                    QFileInfo target_info(ini_path);
                    QDir().mkpath(target_info.absolutePath());
                    if (!QFile::rename(old_ini_path, ini_path)) {
                        if (QFile::copy(old_ini_path, ini_path)) {
                            QFile::remove(old_ini_path);
                        }
                    }
                }
            }
        }
    }

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
