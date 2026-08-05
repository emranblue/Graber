#include "MarkdownTemplateManager.h"
#include "MarkdownUtils.h"
#include "../utils/Utils.h"
#include "../utils/ConfigPaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

MarkdownTemplateManager &MarkdownTemplateManager::instance() {
    static MarkdownTemplateManager mgr;
    return mgr;
}

MarkdownTemplateManager::MarkdownTemplateManager() {
    ConfigPaths::ensureUserConfigFiles();
    file_path_ = ConfigPaths::templatesJsonPath();
}

QString MarkdownTemplateManager::templateFilePath() const {
    return file_path_;
}

void MarkdownTemplateManager::ensureDefaultTemplateFileExists() {
    ConfigPaths::ensureUserConfigFiles();
}

void MarkdownTemplateManager::loadFromDiskIfNeeded() {
    ensureDefaultTemplateFileExists();

    QFileInfo info(file_path_);
    info.refresh();
    if (!info.exists())
        return;

    QDateTime modified = info.lastModified();
    if (templates_.isEmpty() || modified != last_modified_) {
        reload();
    }
}

void MarkdownTemplateManager::reload() {
    templates_.clear();
    display_names_.clear();
    is_diagram_.clear();
    section_order_.clear();

    QFile file(file_path_);
    if (!file.open(QIODevice::ReadOnly)) {
        debugLog(QStringLiteral("MarkdownTemplateManager: cannot open %1").arg(file_path_));
        return;
    }

    QFileInfo info(file_path_);
    last_modified_ = info.lastModified();

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        debugLog(QStringLiteral("MarkdownTemplateManager: invalid JSON in %1").arg(file_path_));
        return;
    }

    const QJsonArray formats = doc.object().value(QStringLiteral("formats")).toArray();
    for (const QJsonValue &v : formats) {
        if (!v.isObject())
            continue;
        const QJsonObject obj = v.toObject();
        const QString key = obj.value(QStringLiteral("key")).toString().trimmed().toLower();
        if (key.isEmpty())
            continue;

        const QString body = obj.value(QStringLiteral("body")).toString();
        const QString display = obj.value(QStringLiteral("display_name")).toString();
        const bool diagram = obj.value(QStringLiteral("is_diagram")).toBool(false);

        templates_[key] = body;
        is_diagram_[key] = diagram;
        if (!display.isEmpty())
            display_names_[key] = display;
        if (!section_order_.contains(key))
            section_order_.append(key);
    }
}

QList<TemplateFormatInfo> MarkdownTemplateManager::getFormatList() {
    loadFromDiskIfNeeded();

    QList<TemplateFormatInfo> list;

    QStringList defaultOrder = {
        QStringLiteral("bullet"),
        QStringLiteral("heading"),
        QStringLiteral("subheading"),
        QStringLiteral("timeline"),
        QStringLiteral("paragraph")
    };

    auto addKey = [&](const QString &key) {
        if (is_diagram_.value(key, false))
            return;
        if (key == QStringLiteral("flowchart") || key == QStringLiteral("sequence") ||
            key == QStringLiteral("mindmap") || key == QStringLiteral("class")) {
            return;
        }

        TemplateFormatInfo info;
        info.key = key;
        info.isDiagram = false;

        if (display_names_.contains(key)) {
            info.displayName = display_names_[key];
        } else if (key == QStringLiteral("bullet")) {
            info.displayName = QStringLiteral("বুলেট পয়েন্ট (Point)");
        } else if (key == QStringLiteral("heading") || key == QStringLiteral("h2")) {
            info.displayName = QStringLiteral("প্রধান শিরোনাম (Heading - Red)");
        } else if (key == QStringLiteral("subheading") || key == QStringLiteral("h3")) {
            info.displayName = QStringLiteral("উপ-শিরোনাম (Subheading - Blue)");
        } else if (key == QStringLiteral("timeline")) {
            info.displayName = QStringLiteral("মাইন্ড ম্যাপ (Timeline Mind Map)");
        } else if (key == QStringLiteral("paragraph")) {
            info.displayName = QStringLiteral("প্যারাগ্রাফ (Paragraph)");
        } else {
            QString formatted = key;
            if (!formatted.isEmpty())
                formatted[0] = formatted[0].toUpper();
            info.displayName = formatted;
        }

        for (const auto &existing : list) {
            if (existing.key == info.key)
                return;
        }
        list.append(info);
    };

    for (const auto &k : defaultOrder) {
        if (templates_.contains(k) || section_order_.contains(k))
            addKey(k);
    }
    for (const auto &k : section_order_)
        addKey(k);

    return list;
}

QString MarkdownTemplateManager::getFormatKeyForIndex(int formatIndex) {
    QList<TemplateFormatInfo> list = getFormatList();
    if (formatIndex >= 0 && formatIndex < list.size())
        return list[formatIndex].key;

    if (formatIndex == 0) return QStringLiteral("bullet");
    if (formatIndex == 1) return QStringLiteral("heading");
    if (formatIndex == 2) return QStringLiteral("subheading");
    if (formatIndex == 3) return QStringLiteral("timeline");
    if (formatIndex == 4) return QStringLiteral("paragraph");
    return QStringLiteral("bullet");
}

QString MarkdownTemplateManager::getDefaultTemplate(const QString &key) const {
    const QString k = key.toLower().trimmed();
    if (k == QStringLiteral("bullet") || k == QStringLiteral("point"))
        return QStringLiteral("▣ {content}\n");
    if (k == QStringLiteral("heading") || k == QStringLiteral("h2") || k == QStringLiteral("main_heading")) {
        return QStringLiteral(
            "\n<h2 id=\"{slug}\" data-section=\"{section}\" "
            "style=\"color: #e74c3c; font-weight: bold; font-style: italic; margin-bottom: 5px;\">"
            "{content}</h2>\n");
    }
    if (k == QStringLiteral("subheading") || k == QStringLiteral("h3")) {
        return QStringLiteral(
            "\n<h3 id=\"{slug}\" "
            "style=\"color: #2980b9; font-weight: bold; font-style: italic; "
            "margin-top: 10px; margin-bottom: 5px;\">{content}</h3>\n");
    }
    if (k == QStringLiteral("timeline")) {
        return QStringLiteral(
            "<div class=\"timeline-item\" style=\"border-left: 2px dashed {color}; margin-left: 20px; "
            "padding-left: 20px; padding-bottom: 12px; position: relative;\">"
            "<span style=\"position: absolute; left: -2px; top: 18px; width: 12px; height: 2px; "
            "background-color: {color};\"></span>"
            "<span style=\"position: absolute; left: 8px; top: 13px; color: {color}; font-size: 10px; "
            "line-height: 1;\">➤</span>"
            "<div style=\"background-color: {color_alpha}; border: 1px solid {color}; border-radius: 6px; "
            "padding: 8px 12px; display: inline-block; box-shadow: 1px 1px 3px rgba(0,0,0,0.05); "
            "margin-left: 10px;\">"
            "<span style=\"color: {color}; font-weight: 600; font-family: 'Segoe UI', 'Kalpurush', "
            "sans-serif; font-size: 16px;\">{content}</span></div></div>\n");
    }
    if (k == QStringLiteral("paragraph") || k == QStringLiteral("para")) {
        return QStringLiteral(
            "<p class=\"paragraph-item\" style=\"color: #2f3640; line-height: 1.6; "
            "font-family: 'Segoe UI', 'Kalpurush', sans-serif; margin-bottom: 10px; "
            "text-align: justify;\">{content}</p>\n");
    }
    if (k == QStringLiteral("quote")) {
        return QStringLiteral(
            "<blockquote style=\"border-left: 4px solid #3498db; margin: 10px 0; padding: 8px 15px; "
            "background: #f8f9fa; font-style: italic; color: #555;\">💬 {content}</blockquote>\n");
    }
    if (k == QStringLiteral("callout")) {
        return QStringLiteral(
            "<div style=\"background: #e1f5fe; border: 1px solid #81d4fa; border-radius: 6px; "
            "padding: 10px 15px; margin: 10px 0; color: #0277bd;\">📌 <b>Notice:</b> {content}</div>\n");
    }
    if (k == QStringLiteral("flowchart") || k == QStringLiteral("flow")) {
        return QStringLiteral(
            "```mermaid\nflowchart TD\n    Start([Start]) --> Action[\"{{CONTENT}}\"]\n"
            "    Action --> End([End])\n```\n");
    }
    if (k == QStringLiteral("sequence")) {
        return QStringLiteral(
            "```mermaid\nsequenceDiagram\n    autonumber\n    User->>System: {{CONTENT}}\n"
            "    System-->>User: Response\n```\n");
    }
    if (k == QStringLiteral("mindmap")) {
        return QStringLiteral(
            "```mermaid\nmindmap\n  root((Main Topic))\n    SubTopic1\n      {{CONTENT}}\n```\n");
    }
    if (k == QStringLiteral("class")) {
        return QStringLiteral(
            "```mermaid\nclassDiagram\n    class NoteNode {\n        +String content\n    }\n"
            "    NoteNode : {{CONTENT}}\n```\n");
    }
    return QStringLiteral("{content}");
}

QString MarkdownTemplateManager::getTemplate(const QString &key) {
    loadFromDiskIfNeeded();
    const QString k = key.toLower().trimmed();

    if (templates_.contains(k) && !templates_[k].trimmed().isEmpty())
        return templates_[k];

    if (k == QStringLiteral("h2") || k == QStringLiteral("main_heading")) {
        if (templates_.contains(QStringLiteral("heading")))
            return templates_[QStringLiteral("heading")];
    } else if (k == QStringLiteral("h3")) {
        if (templates_.contains(QStringLiteral("subheading")))
            return templates_[QStringLiteral("subheading")];
    } else if (k == QStringLiteral("point")) {
        if (templates_.contains(QStringLiteral("bullet")))
            return templates_[QStringLiteral("bullet")];
    } else if (k == QStringLiteral("para")) {
        if (templates_.contains(QStringLiteral("paragraph")))
            return templates_[QStringLiteral("paragraph")];
    }

    return getDefaultTemplate(k);
}

QString MarkdownTemplateManager::formatContent(int formatIndex, const QString &rawText, const QString &sectionSlug) {
    const QString key = getFormatKeyForIndex(formatIndex);
    return formatByKey(key, rawText, sectionSlug);
}

QString MarkdownTemplateManager::formatByKey(const QString &formatKey, const QString &rawText, const QString &sectionSlug) {
    const QString text = escapeHtml(rawText.trimmed());
    if (text.isEmpty())
        return {};

    const QString k = formatKey.toLower().trimmed();
    QString tmpl = getTemplate(k);
    if (tmpl.isEmpty())
        tmpl = getDefaultTemplate(k);

    const QString slug = QString::fromStdString(MarkdownUtils::generate_slug(rawText.trimmed()));
    const QString sec = sectionSlug.isEmpty() ? QStringLiteral("others") : escapeHtml(sectionSlug);
    const QString color = get_random_beautiful_color();
    const QString colorAlpha = color + QStringLiteral("0f");

    QString result = tmpl;

    if (result.contains(QStringLiteral("{content}")) || result.contains(QStringLiteral("{text}")) ||
        result.contains(QStringLiteral("{slug}")) || result.contains(QStringLiteral("{section}")) ||
        result.contains(QStringLiteral("{color}")) || result.contains(QStringLiteral("{color_alpha}")) ||
        result.contains(QStringLiteral("{raw_content}"))) {

        result.replace(QStringLiteral("{content}"), text);
        result.replace(QStringLiteral("{text}"), text);
        result.replace(QStringLiteral("{raw_content}"), rawText.trimmed());
        result.replace(QStringLiteral("{raw_text}"), rawText.trimmed());
        result.replace(QStringLiteral("{slug}"), escapeHtml(slug));
        result.replace(QStringLiteral("{section}"), sec);
        result.replace(QStringLiteral("{color}"), color);
        result.replace(QStringLiteral("{color_alpha}"), colorAlpha);
    } else {
        if (k == QStringLiteral("heading") || k == QStringLiteral("h2")) {
            result = result.arg(escapeHtml(slug), sec, text);
        } else if (k == QStringLiteral("subheading") || k == QStringLiteral("h3")) {
            result = result.arg(escapeHtml(slug), text);
        } else if (k == QStringLiteral("timeline")) {
            result = result.arg(color, colorAlpha, text);
        } else {
            result = result.arg(text);
        }
    }

    return result;
}
