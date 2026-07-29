#ifndef CLIPBOARDGRABBER_H
#define CLIPBOARDGRABBER_H

#include <QWidget>
#include <QSet>
#include <QList>
#include <QCloseEvent>
#include "Types.h"
#include "interfaces/IClipboardMonitor.h"
#include "ClipboardMonitor.h"
#include "NoteService.h"
#include "ShortcutManager.h"
#include "ClipboardGrabberUI.h"

class ClipboardGrabber : public QWidget {
    Q_OBJECT

public:
    explicit ClipboardGrabber(QWidget *parent = nullptr);
    ~ClipboardGrabber() override = default;

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
    QList<SectionItem> get_sections_from_ui() const;
    void populate_sections_ui(const QList<SectionItem> &sections);

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

    // Modular Components & Services
    ClipboardGrabberUI ui_;
    IClipboardMonitor *clipboard_monitor_;
    NoteService note_service_;
    ShortcutManager shortcut_manager_;
};

#endif // CLIPBOARDGRABBER_H
