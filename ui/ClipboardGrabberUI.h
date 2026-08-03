#ifndef CLIPBOARDGRABBERUI_H
#define CLIPBOARDGRABBERUI_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>

// Exact text of the "Diagram" entry in format_dropdown. Shared between the
// item that's added to the dropdown and the logic that checks whether it's
// the current selection, so the two can never drift apart. The format
// dropdown itself is the only switch for diagram mode — there is no
// separate checkbox/toggle to keep in sync with it.
inline const QString kDiagramFormatLabel = QStringLiteral("ডায়াগ্রাম (Diagram)");

class ClipboardGrabberUI {
public:
    ClipboardGrabberUI() = default;
    ~ClipboardGrabberUI() = default;

    /**
     * @brief Constructs and initializes all UI components, layouts, and card panels.
     * @param parent Pointer to the target parent QWidget (e.g., ClipboardGrabberWindow).
     */
    void setupUi(QWidget *parent);
    void cycleFormat();

    // =========================================================================
    // DYNAMIC UI PANELS & LAYOUT CONTAINERS
    // =========================================================================
    QWidget *body = nullptr;
    QWidget *controls_bar = nullptr;
    QWidget *capture_extra = nullptr;
    QWidget *diagram_quick_row = nullptr;

    QFrame *subject_card = nullptr;
    QFrame *capture_card = nullptr;
    QFrame *heading_card = nullptr;

    // =========================================================================
    // STATUS INDICATORS & PREVIEWS
    // =========================================================================
    QLabel *status_label = nullptr;
    QLabel *last_captured_label = nullptr;

    // =========================================================================
    // ACTION BUTTONS
    // =========================================================================
    // Core App Controls
    QPushButton *start_button = nullptr;
    QPushButton *stop_button = nullptr;
    QPushButton *wizards_button = nullptr;
    QPushButton *settings_button = nullptr;

    // Subject & Folder Controls
    QPushButton *add_folder_button = nullptr;
    QPushButton *add_subject_button = nullptr;
    QPushButton *toggle_subject_button = nullptr;
    QPushButton *open_file_button = nullptr;

    // Capture & Section Actions
    QPushButton *add_image_button = nullptr;
    QPushButton *add_section_button = nullptr;

    // Target Heading Actions
    QPushButton *select_heading_button = nullptr;
    QPushButton *append_to_heading_button = nullptr;
    QPushButton *inject_heading_button = nullptr;
    QPushButton *shift_heading_button = nullptr;
    QPushButton *delete_heading_button = nullptr;

    // Diagram Actions
    QPushButton *insert_diagram_button = nullptr;

    // =========================================================================
    // DROPDOWNS & SELECTION CONTROLS
    // =========================================================================
    QComboBox *folder_dropdown = nullptr;
    QComboBox *subject_dropdown = nullptr;
    QComboBox *format_dropdown = nullptr;
    QComboBox *section_dropdown = nullptr;
    QComboBox *mode_dropdown = nullptr;
    QComboBox *diagram_dropdown = nullptr;

    // =========================================================================
    // LABELS
    // =========================================================================
    QLabel *heading_label = nullptr;
    QLabel *section_label = nullptr;
    QLabel *mode_label = nullptr;
};

#endif // CLIPBOARDGRABBERUI_H
