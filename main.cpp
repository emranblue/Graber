#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include "ClipboardGrabber.h"
#include "Utils.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // App-wide icon: shows in the window title bar, taskbar/dock, and
    // Alt+Tab switcher on Linux/macOS. On Windows the .exe itself is
    // additionally iconified via resources/graber.rc at compile time.
    app.setWindowIcon(QIcon(":/icons/app.ico"));

    // Global UI Styling — soft neutral canvas, elevated white cards, and a
    // refined control palette. Individual widgets still set their own accent
    // colors (see ClipboardGrabberUI.cpp); this defines the shared shape,
    // spacing and typography so everything reads as one coherent app.
    app.setStyleSheet(
        "QWidget { font-family: 'Segoe UI', 'Kalpurush'; color: #2f3640; }"
        "QMainWindow, #MainWindow, QDialog { background-color: #eef1f7; }"

        // Footer controls strip
        "#controlsBar { background-color: #ffffff; border-top: 1px solid #e3e7ef; }"

        // Cards
        "QFrame#card { background-color: #ffffff; border: 1px solid #e3e7ef; border-radius: 12px; }"
        "QFrame#heroCard { background-color: #ffffff; border: 1px solid #e3e7ef; border-radius: 14px; }"

        "QLabel { color: #2f3640; font-size: 12px; background-color: transparent; }"

        // Buttons: soft rounded, confident weight, gentle hover/press feedback
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

        // Status pill: gray while idle, warm green while actively capturing
        "#status_label { font-size: 14px; font-weight: 700; color: #718093; padding: 10px; background: #f1f2f6; border-radius: 10px; }"
        "QLabel#status_label[running=\"true\"] { color: #16a34a; background: #e7f8ee; }"

        "#last_captured_label { background-color: #f8f9fc; border: 1px solid #e3e7ef; border-radius: 10px; padding: 12px; color: #2f3640; font-style: italic; border-left: 4px solid #487eb0; }"

        // Scrollbars: slim and unobtrusive
        "QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #ccd3e0; border-radius: 5px; min-height: 24px; }"
        "QScrollBar::handle:vertical:hover { background: #aeb8cc; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
    );

    // Add the Kalpurush font
    int fontId = QFontDatabase::addApplicationFont(":/Kalpurush.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QFont font(fontFamilies.at(0));
            app.setFont(font);
        }
    }

    // Add the Feather font dynamically
    int featherFontId = QFontDatabase::addApplicationFont(":/feather.ttf");
    if (featherFontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(featherFontId);
        if (!fontFamilies.isEmpty()) {
            set_feather_font_family(fontFamilies.at(0));
        }
    }

    ClipboardGrabber window;
    window.show();

    return app.exec();
}
