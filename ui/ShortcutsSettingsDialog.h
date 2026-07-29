#ifndef SHORTCUTSSETTINGSDIALOG_H
#define SHORTCUTSSETTINGSDIALOG_H

#include <QDialog>
#include <QList>
#include <QKeySequenceEdit>
#include "Types.h"

class ShortcutsSettingsDialog : public QDialog {
    Q_OBJECT
public:
    ShortcutsSettingsDialog(QList<ShortcutConfig> &configs, QWidget *parent = nullptr);

private slots:
    void on_reset();
    void on_save();

private:
    QList<ShortcutConfig> &configs_;
    QList<QKeySequenceEdit*> edits_;
};

#endif // SHORTCUTSSETTINGSDIALOG_H
