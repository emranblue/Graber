#ifndef CLIPBOARDGRABBERUI_H
#define CLIPBOARDGRABBERUI_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

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
};

#endif // CLIPBOARDGRABBERUI_H
