#ifndef MARKDOWNTEMPLATEMANAGER_H
#define MARKDOWNTEMPLATEMANAGER_H

#include <QString>
#include <QMap>
#include <QList>
#include <QDateTime>

struct TemplateFormatInfo {
    QString key;          // Format key (e.g. "bullet", "heading", "quote")
    QString displayName;  // UI display label (e.g. "Quote Block (উদ্ধৃতি)")
    bool isDiagram;       // true if this key is a Mermaid diagram skeleton
};

/**
 * Manages Markdown formatting templates loaded from
 * ~/GraberNotes/config/templates.json (seeded from project config/).
 * Users can edit the JSON while Graber is running to customize formats.
 */
class MarkdownTemplateManager {
public:
    static MarkdownTemplateManager &instance();

    /** Path to the template JSON on disk (~/GraberNotes/config/templates.json). */
    QString templateFilePath() const;

    /** Ensures user config JSON exists (copies from embedded resource if needed). */
    void ensureDefaultTemplateFileExists();

    /** Returns all available format options for the UI dropdown. */
    QList<TemplateFormatInfo> getFormatList();

    /** Returns format key corresponding to a numeric format index. */
    QString getFormatKeyForIndex(int formatIndex);

    /**
     * Gets a template string by section key (e.g., "bullet", "heading", "subheading",
     * "timeline", "paragraph", "quote", "flowchart", etc.).
     */
    QString getTemplate(const QString &key);

    /**
     * Formats content using numeric index (0=bullet, 1=heading, 2=subheading, 3=timeline, 4=paragraph, 5+=custom).
     */
    QString formatContent(int formatIndex, const QString &rawText, const QString &sectionSlug = QString());

    /**
     * Formats content using format key string (e.g. "bullet", "heading", "quote", "callout").
     */
    QString formatByKey(const QString &formatKey, const QString &rawText, const QString &sectionSlug = QString());

    /** Forces a reload from disk. */
    void reload();

private:
    MarkdownTemplateManager();
    void loadFromDiskIfNeeded();
    QString getDefaultTemplate(const QString &key) const;

    QMap<QString, QString> templates_;
    QMap<QString, QString> display_names_;
    QMap<QString, bool> is_diagram_;
    QList<QString> section_order_;
    QDateTime last_modified_;
    QString file_path_;
};

#endif // MARKDOWNTEMPLATEMANAGER_H
