#include "section_manager.h"
#include "../../utilities/file_handler.h"
#include "../../utilities/logger.h"
#include "../../utilities/slug_generator.h"
#include "../../core/section_definitions.h"
#include <QSettings>
#include <QDir>

SectionManager::SectionManager(const QString &notesDir)
    : notes_dir_path_(notesDir) {
}

QString SectionManager::getSectionsIniPath(const QString &subject) const {
    return notes_dir_path_ + QDir::separator() + subject + ".ini";
}

void SectionManager::loadSectionsForSubject(const QString &subject, QComboBox *dropdown) {
    if (!dropdown) return;
    dropdown->clear();
    
    if (subject.isEmpty()) {
        dropdown->addItem("\u0985\u09a8\u09cd\u09af\u09be\u09a8\u09cd\u09af (Others)", "others");
        return;
    }
    
    QString ini_path = getSectionsIniPath(subject);
    
    if (!FileHandler::fileExists(ini_path)) {
        dropdown->addItem("\u0985\u09a8\u09cd\u09af\u09be\u09a8\u09cd\u09af (Others)", "others");
        saveSectionsForSubject(subject, dropdown);
        return;
    }
    
    QSettings settings(ini_path, QSettings::IniFormat);
    settings.beginGroup("Sections");
    
    QString order_str = settings.value("_order").toString();
    QStringList order = order_str.split(',', Qt::SkipEmptyParts);
    
    if (!order.isEmpty()) {
        for (const QString &slug : order) {
            QString trimmed_slug = slug.trimmed();
            if (settings.contains(trimmed_slug)) {
                QString display_name = settings.value(trimmed_slug).toString();
                dropdown->addItem(display_name, trimmed_slug);
            }
        }
    }
    
    settings.endGroup();
    
    // Ensure "others" exists
    bool has_others = false;
    for (int i = 0; i < dropdown->count(); ++i) {
        if (dropdown->itemData(i).toString() == "others") {
            has_others = true;
            break;
        }
    }
    if (!has_others) {
        dropdown->addItem("\u0985\u09a8\u09cd\u09af\u09be\u09a8\u09cd\u09af (Others)", "others");
    }
}

void SectionManager::saveSectionsForSubject(const QString &subject, QComboBox *dropdown) {
    if (subject.isEmpty() || !dropdown) return;
    
    QString ini_path = getSectionsIniPath(subject);
    QFileInfo fileInfo(ini_path);
    QDir().mkpath(fileInfo.absolutePath());
    
    QSettings settings(ini_path, QSettings::IniFormat);
    settings.beginGroup("Sections");
    settings.remove("");
    
    QStringList order;
    for (int i = 0; i < dropdown->count(); ++i) {
        QString slug = dropdown->itemData(i).toString();
        QString display_name = dropdown->itemText(i);
        order.append(slug);
        settings.setValue(slug, display_name);
    }
    settings.setValue("_order", order.join(","));
    settings.endGroup();
    
    Logger::info(QString("Sections saved for: %1").arg(subject));
}

void SectionManager::addCustomSection(const QString &sectionName, QComboBox *dropdown) {
    if (sectionName.isEmpty() || !dropdown) return;
    
    QString slug = QString::fromStdString(SlugGenerator::generate(sectionName));
    
    // Check if already exists
    int found_idx = -1;
    for (int i = 0; i < dropdown->count(); ++i) {
        if (dropdown->itemData(i).toString() == slug) {
            found_idx = i;
            break;
        }
    }
    
    if (found_idx == -1) {
        dropdown->addItem(QString("%1 (%2)").arg(sectionName, slug.toUpper()), slug);
        dropdown->setCurrentIndex(dropdown->count() - 1);
        Logger::info(QString("Custom section added: %1").arg(sectionName));
    } else {
        dropdown->setCurrentIndex(found_idx);
    }
}
