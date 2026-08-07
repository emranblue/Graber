#ifndef INJECTFROMNOTEDIALOG_H
#define INJECTFROMNOTEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <functional>
#include "Types.h"

/**
 * Inject picker (NOT clipboard):
 *  1) Select another .md under GraberNotes
 *  2) Select a heading/subheading from that file
 *  3) Accept → caller runs injectSubheadingFromNote()
 */
class InjectFromNoteDialog : public QDialog {
    Q_OBJECT
public:
    using ParseFn = std::function<QList<NoteItem>(const QString &absPath)>;

    InjectFromNoteDialog(const QString &notesDir,
                         const QString &currentTargetFile,
                         ParseFn parseFn,
                         QWidget *parent = nullptr);

    QString sourceFilePath() const { return source_file_; }
    QString sourceSlug() const { return source_slug_; }
    QString sourceTitle() const { return source_title_; }

private slots:
    void onFileFilterChanged(const QString &text);
    void onFileSelected();
    void onHeadingDoubleClicked(QListWidgetItem *item);
    void onInjectClicked();

private:
    void scanNoteFiles();
    void populateFileList(const QString &filter);
    void loadHeadingsForFile(const QString &absPath);

    QString notes_dir_;
    QString current_target_;
    QString source_file_;
    QString source_slug_;
    QString source_title_;

    QStringList all_rel_paths_;
    QStringList all_abs_paths_;

    QLineEdit *file_filter_ = nullptr;
    QListWidget *file_list_ = nullptr;
    QListWidget *heading_list_ = nullptr;
    QLabel *status_ = nullptr;
    ParseFn parse_fn_;
};

#endif // INJECTFROMNOTEDIALOG_H
