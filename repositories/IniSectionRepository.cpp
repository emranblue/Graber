#include "IniSectionRepository.h"
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QSet>

namespace {

// Recover section list from a note's existing <h2 data-section="..."> markers
// (and <!-- section:slug --> markdown markers). Used when the subject's .ini
// is missing or empty so the dropdown / TOC are not left blank.
QList<SectionItem> recoverSectionsFromNote(const QString &notesDirPath, const QString &subjectName) {
    QList<SectionItem> recovered;
    if (subjectName.isEmpty()) return recovered;

    const QString md_path = notesDirPath + QDir::separator() + subjectName + ".md";
    QFile file(md_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return recovered;

    QTextStream in(&file);
    const QString content = in.readAll();
    file.close();

    QSet<QString> seen;
    // Preserve discovery order so the dropdown is stable.
    QStringList order;

    QRegularExpression h2_rx(
        QStringLiteral("<h2[^>]*data-section=\"([^\"]+)\"[^>]*>"),
        QRegularExpression::CaseInsensitiveOption);
    QRegularExpression md_sec_rx(QStringLiteral("<!--\\s*section:([\\w-]+)\\s*-->"));

    auto consider = [&](const QString &slug) {
        const QString s = slug.trimmed();
        if (s.isEmpty() || s == QLatin1String("others")) return;
        if (seen.contains(s)) return;
        seen.insert(s);
        order.append(s);
    };

    auto it = h2_rx.globalMatch(content);
    while (it.hasNext()) {
        consider(it.next().captured(1));
    }
    auto it2 = md_sec_rx.globalMatch(content);
    while (it2.hasNext()) {
        consider(it2.next().captured(1));
    }

    for (const QString &slug : order) {
        // Display name: humanize the slug (replace hyphens, keep as-is otherwise).
        QString display = slug;
        display.replace(QLatin1Char('-'), QLatin1Char(' '));
        if (!display.isEmpty())
            display[0] = display[0].toUpper();
        recovered.append({display, slug});
    }
    return recovered;
}

} // namespace

QList<SectionItem> IniSectionRepository::loadSectionsForSubject(const QString &notesDirPath, const QString &subjectName) {
    if (subjectName.isEmpty() || subjectName == QStringLiteral("নির্বাচিত নয়")) {
        // No subject selected — there is no built-in section list to fall back to.
        return {};
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

    QList<SectionItem> sections;

    if (QFile::exists(ini_path)) {
        QSettings settings(ini_path, QSettings::IniFormat);
        settings.beginGroup("Sections");
        QString order_str = settings.value("_order", "").toString();

        if (!order_str.isEmpty()) {
            QStringList order = order_str.split(",");
            for (const QString &slug : order) {
                if (slug.trimmed().isEmpty()) continue;
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
    }

    // If the .ini is missing or empty, recover section slugs from the note body
    // so the dropdown / TOC are not left blank for existing notes.
    if (sections.isEmpty()) {
        sections = recoverSectionsFromNote(notesDirPath, subjectName);
        // Persist recovered sections so subsequent loads are fast and stable.
        if (!sections.isEmpty()) {
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
    }

    return sections;
}

void IniSectionRepository::saveSectionsForSubject(const QString &notesDirPath, const QString &subjectName, const QList<SectionItem> &sections) {
    if (subjectName.isEmpty() || subjectName == QStringLiteral("নির্বাচিত নয়")) return;

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
