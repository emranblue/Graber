#ifndef HEADINGREORDERDIALOG_H
#define HEADINGREORDERDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QLabel>
#include <QList>
#include "Types.h"

/**
 * Drag-and-drop reorder dialog for note headings / subheadings.
 *
 * - Main headings are top-level rows (not draggable by default for safety).
 * - Subheadings are children; drag a subheading onto another heading/subheading
 *   to move its full content block in the .md file.
 * - Slugs (ids) are preserved → TOC links and in-page navigation stay valid.
 * - Display numbers (1.1, 1.2, …) refresh after each successful move + TOC rebuild.
 */
class HeadingReorderDialog : public QDialog {
    Q_OBJECT
public:
    explicit HeadingReorderDialog(const QList<NoteItem> &all_headings,
                                  const QString &current_slug,
                                  QWidget *parent = nullptr);

    /** Reload tree from a freshly parsed NoteItem list (call after a successful move). */
    void reload(const QList<NoteItem> &all_headings, const QString &select_slug = QString());

signals:
    /**
     * Emitted when the user drops a subheading.
     * @param sourceSlug  slug of the dragged subheading
     * @param targetSlug  slug to insert AFTER (empty = move to end of file)
     */
    void moveRequested(const QString &sourceSlug, const QString &targetSlug);

private:
    void rebuildTree(const QString &select_slug);
    QTreeWidgetItem *findItemBySlug(const QString &slug) const;

    QList<NoteItem> headings_;
    QTreeWidget *tree_ = nullptr;
    QLabel *hint_label_ = nullptr;
    QLabel *status_label_ = nullptr;
};

#endif // HEADINGREORDERDIALOG_H
