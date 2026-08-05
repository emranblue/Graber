#include "ClipboardGrabber.h"
#include "ActionRegistry.h"
#include "ServiceRegistry.h"
#include "FeatureManager.h"
#include "dialogs/HeadingSelectDialog.h"
#include "dialogs/ShortcutsSettingsDialog.h"
#include "MarkdownUtils.h"
#include "utils/UiAnimator.h"
#include "utils/ConfigPaths.h"

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
    // Seed ~/GraberNotes/config/*.json from embedded resources if missing,
    // then open them in the system default editor for live customization.
    ConfigPaths::ensureUserConfigFiles();

    QMenu menu(this);
    menu.setTitle(QStringLiteral("টুলস (Tools)"));
    menu.setStyleSheet(
        "QMenu {"
        "  background-color: #ffffff;"
        "  color: #1d1d1f;"
        "  border: 1px solid rgba(0,0,0,0.12);"
        "  border-radius: 8px;"
        "  padding: 4px 0px;"
        "}"
        "QMenu::item {"
        "  background-color: transparent;"
        "  padding: 6px 16px;"
        "  border-radius: 4px;"
        "  margin: 2px 4px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #f0f0f5;"
        "  color: #007aff;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background-color: #e5e5ea;"
        "  margin: 4px 8px;"
        "}"
    );

    QAction *openTemplates = menu.addAction(
        QStringLiteral("টেমপ্লেট JSON খুলুন (Open templates.json)"));
    QAction *openShortcuts = menu.addAction(
        QStringLiteral("শর্টকাট JSON খুলুন (Open shortcut_templates.json)"));
    menu.addSeparator();
    QAction *openConfigDir = menu.addAction(
        QStringLiteral("কনফিগ ফোল্ডার খুলুন (Open config folder)"));

    // Keep any registered IWizardFeature entries as secondary tools.
    const auto features = FeatureManager::instance().getAllFeatures();
    if (!features.isEmpty()) {
        menu.addSeparator();
        for (const auto &feat : features) {
            QAction *act = menu.addAction(feat->displayName());
            connect(act, &QAction::triggered, this, [this, feat]() {
                feat->executeWizard(this, &ServiceRegistry::instance());
            });
        }
    }

    connect(openTemplates, &QAction::triggered, this, [this]() {
        const QString path = ConfigPaths::templatesJsonPath();
        if (!ConfigPaths::openPathInSystem(path)) {
            QMessageBox::warning(this, QStringLiteral("টুলস"),
                QStringLiteral("templates.json খোলা যায়নি:\n%1").arg(path));
        } else {
            ui_.last_captured_label->setText(
                QStringLiteral("খোলা হয়েছে: %1").arg(path));
        }
    });
    connect(openShortcuts, &QAction::triggered, this, [this]() {
        const QString path = ConfigPaths::shortcutTemplatesJsonPath();
        if (!ConfigPaths::openPathInSystem(path)) {
            QMessageBox::warning(this, QStringLiteral("টুলস"),
                QStringLiteral("shortcut_templates.json খোলা যায়নি:\n%1").arg(path));
        } else {
            ui_.last_captured_label->setText(
                QStringLiteral("খোলা হয়েছে: %1").arg(path));
        }
    });
    connect(openConfigDir, &QAction::triggered, this, [this]() {
        const QString path = ConfigPaths::configDir();
        if (!ConfigPaths::openPathInSystem(path)) {
            QMessageBox::warning(this, QStringLiteral("টুলস"),
                QStringLiteral("কনফিগ ফোল্ডার খোলা যায়নি:\n%1").arg(path));
        }
    });

    menu.exec(ui_.wizards_button->mapToGlobal(QPoint(0, ui_.wizards_button->height())));
}
