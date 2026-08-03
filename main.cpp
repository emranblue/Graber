
#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include <QAbstractNativeEventFilter>
#include "ClipboardGrabber.h"
#include "utils/Utils.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#endif

// Global Native Event Filter to capture Ctrl+Shift+F across the system
class GlobalHotkeyFilter : public QAbstractNativeEventFilter {
public:
    GlobalHotkeyFilter(ClipboardGrabber *grabber) : m_grabber(grabber) {}

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override {
        Q_UNUSED(result);

#if defined(Q_OS_WIN)
        if (eventType == "windows_generic_MSG") {
            MSG *msg = static_cast<MSG *>(message);
            if (msg->message == WM_HOTKEY && msg->wParam == 1001) { // 1001 = Hotkey ID
                if (m_grabber) {
                    // Call the UI cycle format method on hotkey trigger
                    m_grabber->cycleFormat();
                }
                return true;
            }
        }
#endif
        return false;
    }

private:
    ClipboardGrabber *m_grabber;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // App-wide icon: shows in window title bar, taskbar, and Alt+Tab switcher
    app.setWindowIcon(QIcon(":/icons/app.ico"));

    // Global UI Styling
    app.setStyleSheet(
        "QWidget { font-family: 'Segoe UI', 'Kalpurush'; color: #2f3640; }"
        "QMainWindow, #MainWindow, QDialog { background-color: #eef1f7; }"

        // Footer controls strip
        "#controlsBar { background-color: #ffffff; border-top: 1px solid #e3e7ef; }"

        // Cards
        "QFrame#card { background-color: #ffffff; border: 1px solid #e3e7ef; border-radius: 12px; }"
        "QFrame#heroCard { background-color: #ffffff; border: 1px solid #e3e7ef; border-radius: 14px; }"

        "QLabel { color: #2f3640; font-size: 12px; background-color: transparent; }"

        // Buttons
        "QPushButton { background-color: #487eb0; color: white; border-radius: 8px; padding: 6px 11px; font-weight: 600; font-size: 12px; border: none; min-width: 60px; min-height: 15px; }"
        "QPushButton:hover { background-color: #40739e; }"
        "QPushButton:pressed { background-color: #35618a; padding-top: 7px; padding-bottom: 5px; }"
        "QPushButton:disabled { background-color: #dde3ec; color: #a4acb9; }"
        "QPushButton#primaryActionButton { border-radius: 9px; padding: 9px 12px; font-size: 13px; }"
        "QPushButton#secondaryButton { background-color: #718093; }"
        "QPushButton#secondaryButton:hover { background-color: #636e72; }"

        // Inputs
        "QComboBox { border: 1.5px solid #dfe4ec; border-radius: 7px; padding: 4px 8px; background: white; color: #2f3640; min-height: 24px; }"
        "QComboBox:hover { border: 1.5px solid #a9c3dd; }"
        "QComboBox:focus { border: 1.5px solid #487eb0; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
        "QComboBox QAbstractItemView { background: white; color: #2f3640; border: 1px solid #e3e7ef; border-radius: 8px; selection-background-color: #487eb0; selection-color: white; padding: 4px; outline: none; }"
        "QLineEdit { background: white; color: #2f3640; padding: 6px 8px; border: 1.5px solid #dfe4ec; border-radius: 7px; }"
        "QLineEdit:focus { border: 1.5px solid #487eb0; }"

        // Status pill
        "#status_label { font-size: 14px; font-weight: 700; color: #718093; padding: 10px; background: #f1f2f6; border-radius: 10px; }"
        "QLabel#status_label[running=\"true\"] { color: #16a34a; background: #e7f8ee; }"

        "#last_captured_label { background-color: #f8f9fc; border: 1px solid #e3e7ef; border-radius: 10px; padding: 12px; color: #2f3640; font-style: italic; border-left: 4px solid #487eb0; }"

        // Scrollbars
        "QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #ccd3e0; border-radius: 5px; min-height: 24px; }"
        "QScrollBar::handle:vertical:hover { background: #aeb8cc; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
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

    ClipboardGrabber window;

#if defined(Q_OS_WIN)
    // Register Ctrl+Shift+F globally on Windows (VK_F = 0x46, MOD_CONTROL | MOD_SHIFT = 0x0002 | 0x0004)
    RegisterHotKey((HWND)window.winId(), 1001, MOD_CONTROL | MOD_SHIFT, 0x46);
#endif

    GlobalHotkeyFilter hotkeyFilter(&window);
    app.installNativeEventFilter(&hotkeyFilter);

    window.show();

    int execResult = app.exec();

#if defined(Q_OS_WIN)
    UnregisterHotKey((HWND)window.winId(), 1001);
#endif

    return execResult;
}
