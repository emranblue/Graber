
#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include "ClipboardGrabber.h"
#include "utils/Utils.h"
#include "utils/CrashGuard.h"
#include "utils/UiAnimator.h"

// Note: Ctrl+Shift+F ("cycle format") used to also be registered here as a
// second, independent global hotkey (via RegisterHotKey/GlobalHotkeyFilter on
// Windows), on top of the one ShortcutManager's GlobalHotkeyListener already
// registers for the "toggle_format" action. With both active, a single
// keypress fired both handlers, advancing the format dropdown by 2 indices
// instead of 1 (so with 6 items it only ever landed on 3 of them). That
// duplicate registration has been removed; ShortcutManager is the single
// owner of this shortcut on every platform.

int main(int argc, char *argv[]) {
    // Process-wide terminate + Qt message handlers (log to ~/GraberNotes/debug.log).
    CrashGuard::installGlobalHandlers();

    QApplication app(argc, argv);

    // App-wide icon: shows in window title bar, taskbar, and Alt+Tab switcher
    app.setWindowIcon(get_app_icon());

    // Soft Mac-like global styling: airy neutrals, large radii, glossy cards.
    app.setStyleSheet(
        "QWidget { font-family: 'SF Pro Text', 'Segoe UI', 'Kalpurush', system-ui, sans-serif;"
        "  color: #1d1d1f; font-size: 13px; }"
        "QMainWindow, #MainWindow, QDialog {"
        "  background-color: #f5f5f7; }"

        "#controlsBar {"
        "  background-color: rgba(255,255,255,0.92);"
        "  border-top: 1px solid rgba(0,0,0,0.06); }"

        "QFrame#card {"
        "  background-color: #ffffff;"
        "  border: 1px solid rgba(0,0,0,0.06);"
        "  border-radius: 16px; }"
        "QFrame#heroCard {"
        "  background-color: #ffffff;"
        "  border: 1px solid rgba(0,0,0,0.06);"
        "  border-radius: 18px; }"

        "QLabel { color: #1d1d1f; font-size: 12px; background-color: transparent; }"

        "QPushButton {"
        "  background-color: #007aff; color: white;"
        "  border-radius: 10px; padding: 7px 14px;"
        "  font-weight: 600; font-size: 12px; border: none;"
        "  min-width: 64px; min-height: 16px; }"
        "QPushButton:hover { background-color: #0066d6; }"
        "QPushButton:pressed { background-color: #0055b3; padding-top: 8px; padding-bottom: 6px; }"
        "QPushButton:disabled { background-color: #e5e5ea; color: #8e8e93; }"
        "QPushButton#primaryActionButton { border-radius: 11px; padding: 10px 14px; font-size: 13px; }"
        "QPushButton#secondaryButton { background-color: #8e8e93; }"
        "QPushButton#secondaryButton:hover { background-color: #6c6c70; }"

        "QComboBox {"
        "  border: 1px solid rgba(0,0,0,0.12); border-radius: 10px;"
        "  padding: 5px 10px; background: #ffffff; color: #1d1d1f; min-height: 26px; }"
        "QComboBox:hover { border: 1px solid rgba(0,122,255,0.45); }"
        "QComboBox:focus { border: 1.5px solid #007aff; }"
        "QComboBox::drop-down { border: none; width: 22px; }"
        "QComboBox QAbstractItemView {"
        "  background: #ffffff; color: #1d1d1f;"
        "  border: 1px solid rgba(0,0,0,0.08); border-radius: 12px;"
        "  selection-background-color: #007aff; selection-color: white;"
        "  padding: 6px; outline: none; }"
        "QLineEdit {"
        "  background: #ffffff; color: #1d1d1f;"
        "  padding: 7px 10px; border: 1px solid rgba(0,0,0,0.12);"
        "  border-radius: 10px; }"
        "QLineEdit:focus { border: 1.5px solid #007aff; }"

        "#status_label {"
        "  font-size: 14px; font-weight: 700; color: #8e8e93;"
        "  padding: 12px; background: #f2f2f7; border-radius: 12px; }"
        "QLabel#status_label[running=\"true\"] {"
        "  color: #248a3d; background: #e8f8ed; }"

        "#last_captured_label {"
        "  background-color: #fafafa; border: 1px solid rgba(0,0,0,0.06);"
        "  border-radius: 12px; padding: 12px; color: #3a3a3c;"
        "  font-style: italic; border-left: 4px solid #007aff; }"

        "QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }"
        "QScrollBar::handle:vertical {"
        "  background: rgba(0,0,0,0.18); border-radius: 5px; min-height: 28px; }"
        "QScrollBar::handle:vertical:hover { background: rgba(0,0,0,0.28); }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"

        "QCheckBox { spacing: 8px; color: #1d1d1f; }"
        "QToolTip {"
        "  background-color: #1d1d1f; color: #f5f5f7;"
        "  border: none; border-radius: 8px; padding: 6px 10px; }"
    );


    // Add Kalpurush font
    int fontId = QFontDatabase::addApplicationFont(":/Kalpurush.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QFont font(fontFamilies.at(0));
            app.setFont(font);
        }
    }

    // Add Feather font
    int featherFontId = QFontDatabase::addApplicationFont(":/feather.ttf");
    if (featherFontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(featherFontId);
        if (!fontFamilies.isEmpty()) {
            set_feather_font_family(fontFamilies.at(0));
        }
    }

    int exitCode = 1;
    CrashGuard::safeCall([&]() {
        ClipboardGrabber window;
        UiAnimator::fadeInWindow(&window);
        exitCode = app.exec();
    }, QStringLiteral("main.eventLoop"));

    return exitCode;
}
