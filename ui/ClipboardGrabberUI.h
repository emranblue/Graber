#ifndef CLIPBOARDGRABBERUI_H
#define CLIPBOARDGRABBERUI_H

#include <QWidget>
class MacTitleBar;
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QFrame>
#include <QScrollArea>

#include "panels/StatusPanel.h"
#include "panels/SubjectPanel.h"
#include "panels/CapturePanel.h"
#include "panels/HeadingPanel.h"
#include "panels/ControlsBar.h"

// Re-export so existing code that included ClipboardGrabberUI.h still sees it.
using ::kDiagramFormatLabel;

/**
 * Thin compositor: owns the five modular panels and exposes the same
 * flat widget pointers the controller already uses. Layout construction
 * lives inside each panel; this class only assembles them.
 */
class ClipboardGrabberUI {
public:
    ClipboardGrabberUI() = default;
    ~ClipboardGrabberUI() = default;

    void setupUi(QWidget *parent);

    // =========================================================================
    // PANEL INSTANCES (primary modular surface)
    // =========================================================================
    StatusPanel  *status_panel   = nullptr;
    SubjectPanel *subject_panel  = nullptr;
    CapturePanel *capture_panel  = nullptr;
    HeadingPanel *heading_panel  = nullptr;
    ControlsBar  *controls_bar_w = nullptr; // avoid clash with controls_bar pointer below

    // =========================================================================
    // LAYOUT CONTAINERS (kept for fit_window_to_content / visibility)
    // =========================================================================
    MacTitleBar *title_bar = nullptr;
    QWidget *body = nullptr;
    QWidget *controls_bar = nullptr;   // alias → controls_bar_w
    QWidget *capture_extra = nullptr;  // alias → capture_panel->captureExtra()
    QWidget *diagram_quick_row = nullptr;

    QFrame *subject_card = nullptr;    // alias → subject_panel
    QFrame *capture_card = nullptr;    // alias → capture_panel
    QFrame *heading_card = nullptr;    // alias → heading_panel

    // =========================================================================
    // FLAT ACCESSORS (backward-compatible with ClipboardGrabber.cpp)
    // =========================================================================
    QLabel *status_label = nullptr;
    QLabel *last_captured_label = nullptr;

    QPushButton *start_button = nullptr;
    QPushButton *stop_button = nullptr;
    QPushButton *wizards_button = nullptr;
    QPushButton *settings_button = nullptr;

    QPushButton *add_folder_button = nullptr;
    QPushButton *add_subject_button = nullptr;
    QPushButton *toggle_subject_button = nullptr;
    QPushButton *open_file_button = nullptr;

    QPushButton *add_image_button = nullptr;
    QPushButton *add_section_button = nullptr;

    QPushButton *select_heading_button = nullptr;
    QPushButton *append_to_heading_button = nullptr;
    QPushButton *inject_heading_button = nullptr;
    QPushButton *shift_heading_button = nullptr;
    QPushButton *delete_heading_button = nullptr;

    QPushButton *insert_diagram_button = nullptr;

    QComboBox *folder_dropdown = nullptr;
    QComboBox *subject_dropdown = nullptr;
    QComboBox *format_dropdown = nullptr;
    QComboBox *section_dropdown = nullptr;
    QComboBox *mode_dropdown = nullptr;
    QComboBox *diagram_dropdown = nullptr;

    QLabel *heading_label = nullptr;
    QLabel *section_label = nullptr;
    QLabel *mode_label = nullptr;
};

#endif // CLIPBOARDGRABBERUI_H
