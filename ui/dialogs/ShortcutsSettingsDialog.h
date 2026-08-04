#ifndef SHORTCUTSSETTINGSDIALOG_H
#define SHORTCUTSSETTINGSDIALOG_H

#include <QDialog>
#include <QList>
#include <QKeySequenceEdit>
#include <QCheckBox>
#include "Types.h"

/**
 * Edit keyboard shortcuts + per-action enable switches.
 * Also toggles global (system-wide) hotkeys and the diagram panel.
 */
class ShortcutsSettingsDialog : public QDialog {
    Q_OBJECT
public:
    ShortcutsSettingsDialog(QList<ShortcutConfig> &configs,
                             bool &globalHotkeysEnabled,
                             bool globalHotkeysSupported,
                             bool &diagramPanelEnabled,
                             QWidget *parent = nullptr);

private slots:
    void on_reset();
    void on_save();

private:
    QList<ShortcutConfig> &configs_;
    QList<QKeySequenceEdit*> edits_;
    QList<QCheckBox*> enable_checks_;
    bool &global_hotkeys_enabled_;
    QCheckBox *global_hotkeys_checkbox_;
    bool &diagram_panel_enabled_;
    QCheckBox *diagram_panel_checkbox_;
};

#endif // SHORTCUTSSETTINGSDIALOG_H
