#ifndef CLIPBOARDGRABBER_H
#define CLIPBOARDGRABBER_H

#include <QWidget>
#include <QSet>
#include <QList>
#include <QCloseEvent>
#include "Types.h"
#include "ClipboardMonitor.h"
#include "NoteRepository.h"
#include "ShortcutManager.h"
#include "ClipboardGrabberUI.h"

class ClipboardGrabber : public QWidget {
    Q_OBJECT

public:
    ClipboardGrabber(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void toggle_subject();
    void start_monitoring();
    void stop_monitoring();
    void add_subject();
    void add_folder();
    void handle_text_captured(const QString &text);
    void handle_image_captured(const QImage &image);
    void on_subject_changed(const QString &text);
    void inject_heading_from_clipboard();
    void open_selected_file();
    void add_clipboard_image();
    void update_button_states();
    void open_heading_select_dialog();
    void add_section();
    void shift_selected_heading_section();
    void manual_append_to_heading();
    void delete_selected_heading_section();
    void open_settings_dialog();
    void trigger_shortcut_action(const QString &action_id);

private:
    void save_sections_for_subject(const QString &subject_name);
    void load_sections_for_subject(const QString &subject_name);
    void populate_subjects_from_disk();
    QString get_current_target_file();
    void update_status_label();
    void write_to_file(const QString &processed_text, const QString &section = "others");
    void populate_headings_from_file();

    // State Variables
    bool is_running_;
    QString last_date_;
    QSet<QString> custom_added_sections_;

    // Target Heading selection state
    QString selected_heading_slug_;
    QString selected_heading_title_;
    QList<NoteItem> all_headings_;

    // Modular Components & UI
    ClipboardGrabberUI ui_;
    ClipboardMonitor *clipboard_monitor_;
    NoteRepository note_repository_;
    ShortcutManager shortcut_manager_;
};

#endif // CLIPBOARDGRABBER_H
