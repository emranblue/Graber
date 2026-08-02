#ifndef CLIPBOARDGRABBERUI_H
#define CLIPBOARDGRABBERUI_H

#include "core/QtFixes.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSize>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QColor>
#include <QScrollArea>

class ClipboardGrabberUI {
public:
    void setupUi(QWidget *parent);

    // UI Widgets
    QLabel *status_label;
    QLabel *last_captured_label;
    QPushButton *start_button;
    QPushButton *stop_button;
    QPushButton *add_image_button;
    QComboBox *folder_dropdown;
    QComboBox *subject_dropdown;
    QPushButton *toggle_subject_button;
    QPushButton *add_subject_button;
    QPushButton *add_folder_button;
    QPushButton *open_file_button;
    QComboBox *format_dropdown;
    QLabel *mode_label;
    QComboBox *mode_dropdown;
    QLabel *section_label;
    QComboBox *section_dropdown;
    QPushButton *inject_heading_button;
    QLabel *heading_label;
    QPushButton *select_heading_button;
    QPushButton *append_to_heading_button;
    QPushButton *delete_heading_button;
    QPushButton *shift_heading_button;
    QPushButton *add_section_button;
    QPushButton *settings_button;
    QPushButton *wizards_button;

    // Cards/sections toggled between the full editing view and the minimal
    // "capturing" view (see ClipboardGrabber::start_monitoring/stop_monitoring).
    QFrame *subject_card;
    QFrame *capture_card;
    QFrame *heading_card;
    QWidget *capture_extra;   // section + image controls inside capture_card

    // Top-level regions, kept as members so the window can be sized from
    // their real content. QScrollArea::sizeHint()/minimumSizeHint() are
    // hardcoded by Qt to a small capped value that ignores the scrolled
    // widget entirely, so `body` (what's actually inside the scroll area)
    // must be measured directly instead of going through `scroll_area`.
    QWidget *body;
    QWidget *controls_bar;
};

#endif // CLIPBOARDGRABBERUI_H
