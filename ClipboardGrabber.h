#ifndef CLIPBOARDGRABBER_H
#define CLIPBOARDGRABBER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTimer>
#include <QSet>
#include <QList>
#include <QCloseEvent>
#include "Types.h"

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
    void check_clipboard();
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

private:
    void save_sections_for_subject(const QString &subject_name);
    void load_sections_for_subject(const QString &subject_name);
    void populate_subjects_from_disk();
    void normalize_markdown_file(const QString &file_path);
    QString get_current_target_file();
    void update_status_label();
    void write_image_to_file(const QString &image_filename);
    void write_to_file(const QString &processed_text, const QString &section = "others");
    void update_toc_in_file(const QString &file_path);
    void parse_note_structure(const QString &file_path, QList<NoteItem> &items);
    void populate_headings_from_file();
    bool append_content_to_heading(const QString &file_path, const QString &slug, const QString &processed_text);
    void init_shortcut_configs();
    void load_settings();
    void save_settings();
    void setup_shortcuts();
    void trigger_shortcut_action(const QString &action_id);

    // State Variables
    bool is_running_;
    QString last_simplified_text_;
    QString last_date_;
    QString notes_dir_path_;
    QSet<QString> custom_added_sections_;

    // UI Pointers
    QLabel *status_label_;
    QLabel *last_captured_label_;
    QPushButton *start_button_;
    QPushButton *stop_button_;
    QPushButton *add_image_button_;
    QComboBox *subject_dropdown_;
    QPushButton *toggle_subject_button_;
    QPushButton *add_subject_button_;
    QPushButton *add_folder_button_;
    QPushButton *open_file_button_;
    QComboBox *format_dropdown_;
    QTimer *clipboard_timer_;
    QLabel *mode_label_;
    QComboBox *mode_dropdown_;
    QLabel *section_label_;
    QComboBox *section_dropdown_;
    QPushButton *inject_heading_button_;
    QLabel *heading_label_;
    QPushButton *select_heading_button_;
    QString selected_heading_slug_;
    QString selected_heading_title_;
    QPushButton *append_to_heading_button_;
    QPushButton *delete_heading_button_;
    QList<NoteItem> all_headings_;
    QPushButton *shift_heading_button_;
    QPushButton *add_section_button_;
    QPushButton *settings_button_;
    QList<ShortcutConfig> shortcut_configs_;
};

#endif // CLIPBOARDGRABBER_H
