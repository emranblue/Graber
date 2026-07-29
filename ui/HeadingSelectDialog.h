#ifndef HEADINGSELECTDIALOG_H
#define HEADINGSELECTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QKeyEvent>
#include <QCoreApplication>
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

private:
    void populate_list(const QString &search_text);
    QString highlight_text(const QString &src, const QStringList &keywords);

    const QList<NoteItem> &all_headings_;
    QString selected_slug_;
    QString selected_title_;
    QLineEdit *search_edit_;
    QListWidget *list_widget_;
};

#endif // HEADINGSELECTDIALOG_H
