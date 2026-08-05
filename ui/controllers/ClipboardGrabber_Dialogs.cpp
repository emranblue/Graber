#include "ClipboardGrabber.h"
#include "ActionRegistry.h"
#include "ServiceRegistry.h"
#include "FeatureManager.h"
#include "dialogs/HeadingSelectDialog.h"
#include "dialogs/ShortcutsSettingsDialog.h"
#include "MarkdownUtils.h"
#include "utils/UiAnimator.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QInputDialog>
#include <QSettings>
#include <QDir>
#include <QTimer>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>

void ClipboardGrabber::open_settings_dialog() {
    bool global_on = shortcut_manager_.globalHotkeysEnabled();
    ShortcutsSettingsDialog dlg(shortcut_manager_.configs(), global_on,
        shortcut_manager_.globalHotkeysSupported(), diagram_panel_enabled_, this);

    if (UiAnimator::execDialogSmooth(&dlg) != QDialog::Accepted) return;

    shortcut_manager_.enableGlobalHotkeys(global_on);
    QString path = note_service_.notesDirPath() + QDir::separator() + "settings.ini";
    shortcut_manager_.saveSettings(path);

    QSettings s(path, QSettings::IniFormat);
    s.setValue("General/DiagramPanel", diagram_panel_enabled_);

    shortcut_manager_.setupShortcuts(this);
    apply_diagram_panel_visibility();
    ActionRegistry::instance().updateBoundButtons();
    ui_.status_label->setText("অবস্থা: শর্টকাটসমূহ সফলভাবে সংরক্ষণ করা হয়েছে!");
    QTimer::singleShot(0, this, &ClipboardGrabber::fit_window_to_content);
}

void ClipboardGrabber::open_wizards_dialog() {
    auto features = FeatureManager::instance().getAllFeatures();
    if (features.isEmpty()) {
        QMessageBox::information(this, "উইজার্ড ও টুলস",
            "কোনো এক্সটেনশন বা উইজার্ড ইনস্টল করা নেই।");
        return;
    }

    QMenu menu(this);
    for (const auto &feat : features) {
        QAction *act = menu.addAction(feat->displayName());
        connect(act, &QAction::triggered, this, [this, feat]() {
            feat->executeWizard(this, &ServiceRegistry::instance());
        });
    }
    menu.exec(ui_.wizards_button->mapToGlobal(QPoint(0, ui_.wizards_button->height())));
}
