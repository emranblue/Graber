#ifndef HEADINGSELECTDIALOG_H
#define HEADINGSELECTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QKeyEvent>
#include <QEvent>
#include <QCoreApplication>
#include <QTimer>
#include "Types.h"

class HeadingSelectDialog : public QDialog {
    Q_OBJECT
public:
    HeadingSelectDialog(const QList<NoteItem> &all_headings, const QString &current_slug, QWidget *parent = nullptr);

    QString get_selected_slug() const { return selected_slug_; }
    QString get_selected_title() const { return selected_title_; }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_search_text_changed(const QString &text);
    void on_item_double_clicked(QListWidgetItem *item);
    void on_select_clicked();
    void on_debounce_timer();

private:
    struct SearchResult {
        int index;
        int relevance_score;
        const NoteItem *item;
    };

    void populate_list(const QString &search_text);
    QString highlight_text(const QString &src, const QStringList &keywords);
    int calculate_relevance(const NoteItem &item, const QStringList &keywords);
    bool fuzzy_match(const QString &text, const QString &pattern);

    const QList<NoteItem> &all_headings_;
    QStringList display_ids_; // TOC-style numeric ids (e.g. "1", "1.1"), parallel to all_headings_
    QString selected_slug_;
    QString selected_title_;
    QLineEdit *search_edit_;
    QListWidget *list_widget_;
    QTimer *debounce_timer_;
    QString pending_search_text_;
};

#endif // HEADINGSELECTDIALOG_H
