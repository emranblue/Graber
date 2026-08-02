#ifndef SHORTCUTSSETTINGSDIALOG_H
#define SHORTCUTSSETTINGSDIALOG_H

#include <QDialog>
#include <QList>
#include <QEvent>
#include <QKeySequenceEdit>
#include <QCheckBox>
#include "Types.h"

class ShortcutsSettingsDialog : public QDialog {
    Q_OBJECT
public:
    // globalHotkeysEnabled: current on/off state of system-wide hotkeys, read
    // and written in place so the caller can persist it after exec().
    // globalHotkeysSupported: whether the current platform/session can
    // actually deliver global hotkeys (used to grey out / explain the option).
    ShortcutsSettingsDialog(QList<ShortcutConfig> &configs,
                             bool &globalHotkeysEnabled,
                             bool globalHotkeysSupported,
                             QWidget *parent = nullptr);

private slots:
    void on_reset();
    void on_save();

private:
    QList<ShortcutConfig> &configs_;
    QList<QKeySequenceEdit*> edits_;
    bool &global_hotkeys_enabled_;
    QCheckBox *global_hotkeys_checkbox_;
};

#endif // SHORTCUTSSETTINGSDIALOG_H
