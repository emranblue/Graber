#include "ClipboardGrabber.h"
#include "ActionRegistry.h"
#include "ServiceRegistry.h"

#include <QSettings>
#include <QTimer>
#include <QScreen>
#include <QLayout>
#include <QDir>
#include <QStyle>

ClipboardGrabber::ClipboardGrabber(QWidget *parent) : QWidget(parent) {
    ui_.setupUi(this);

    is_running_ = false;
    is_always_on_top_ = false;
    diagram_panel_enabled_ = true;
    last_date_ = "";

    clipboard_monitor_ = new ClipboardMonitor(this);
    connect(clipboard_monitor_, &IClipboardMonitor::textCaptured,
            this, &ClipboardGrabber::handle_text_captured);
    connect(clipboard_monitor_, &IClipboardMonitor::imageCaptured,
            this, &ClipboardGrabber::handle_image_captured);

    setup_services();
    setup_actions();
    setup_features();

    // ShortcutManager already executes the action once (local QShortcut
    // and/or global hotkey, with its own dedup). Connecting actionTriggered
    // here used to call executeAction a second time, so cycle actions like
    // toggle_format advanced the dropdown by 2 indices per keypress.
    // Leave the signal unconnected for execution; keep the slot as a no-op
    // for ABI/header compatibility.

    const QString settings_path =
        note_service_.notesDirPath() + QDir::separator() + "settings.ini";
    shortcut_manager_.loadSettings(settings_path);
    shortcut_manager_.setupShortcuts(this);

    {
        QSettings s(settings_path, QSettings::IniFormat);
        diagram_panel_enabled_ = s.value("General/DiagramPanel", true).toBool();
    }
    apply_diagram_panel_visibility();
    load_sections_for_subject("");

    // Bind buttons → ActionRegistry
    auto &reg = ActionRegistry::instance();
    reg.bindButton(ui_.start_button, "start");
    reg.bindButton(ui_.stop_button, "stop");
    reg.bindButton(ui_.add_image_button, "add_image");
    reg.bindButton(ui_.toggle_subject_button, "toggle_subject");
    reg.bindButton(ui_.add_subject_button, "new_subject");
    reg.bindButton(ui_.add_folder_button, "add_folder");
    reg.bindButton(ui_.open_file_button, "open_note");
    reg.bindButton(ui_.append_to_heading_button, "append");
    reg.bindButton(ui_.delete_heading_button, "delete");
    reg.bindButton(ui_.inject_heading_button, "inject");
    reg.bindButton(ui_.shift_heading_button, "shift");
    reg.bindButton(ui_.add_section_button, "new_section");
    reg.bindButton(ui_.settings_button, "settings");
    reg.bindButton(ui_.wizards_button, "wizards");
    reg.bindButton(ui_.select_heading_button, "select_heading");
    reg.bindButton(ui_.insert_diagram_button, "insert_diagram");

    connect(ui_.folder_dropdown, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClipboardGrabber::on_folder_changed);
    connect(ui_.subject_dropdown, &QComboBox::currentTextChanged,
            this, [this](const QString &t) { on_subject_changed(t); });
    connect(ui_.format_dropdown, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClipboardGrabber::on_format_changed);

    apply_diagram_format_lock();
    populate_folders_from_disk();
    populate_subjects_from_disk();
    ui_.subject_dropdown->setCurrentIndex(-1);
    update_status_label();

    QTimer::singleShot(0, this, &ClipboardGrabber::fit_window_to_content);
}

void ClipboardGrabber::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
}

void ClipboardGrabber::fit_window_to_content() {
    layout()->invalidate();
    layout()->activate();
    ui_.body->layout()->invalidate();
    ui_.body->layout()->activate();

    const QSize body_hint = ui_.body->sizeHint();
    const QSize controls_hint = ui_.controls_bar->sizeHint();
    const QSize body_min = ui_.body->minimumSizeHint();
    const QSize controls_min = ui_.controls_bar->minimumSizeHint();

    int target_w = qMax(body_hint.width(), controls_hint.width());
    int target_h = body_hint.height() + controls_hint.height();
    int min_w = qMax(body_min.width(), controls_min.width());
    int min_h = body_min.height() + controls_min.height();

    setMinimumSize(qMax(min_w, 360), qMax(min_h, 360));

    if (QScreen *scr = screen()) {
        const QRect avail = scr->availableGeometry();
        target_w = qMin(target_w, avail.width() - 40);
        target_h = qMin(target_h, avail.height() - 40);
    }

    resize(qMax(target_w, minimumWidth()), qMax(target_h, minimumHeight()));
}

void ClipboardGrabber::update_status_label() {
    ui_.status_label->setText(is_running_
        ? "অবস্থা: ক্লিপবোর্ড পর্যবেক্ষণ চালুরত..."
        : "অবস্থা: বন্ধ");
    ui_.status_label->setProperty("running", is_running_);
    ui_.status_label->style()->unpolish(ui_.status_label);
    ui_.status_label->style()->polish(ui_.status_label);
}

void ClipboardGrabber::update_button_states() {
    ActionRegistry::instance().updateBoundButtons();
}

void ClipboardGrabber::toggle_always_on_top() {
    is_always_on_top_ = !is_always_on_top_;
    setWindowFlag(Qt::WindowStaysOnTopHint, is_always_on_top_);
    show();
    ui_.status_label->setText(is_always_on_top_
        ? "অবস্থা: উইন্ডো সর্বদা উপরে পিন করা হয়েছে (Always On Top: ON)"
        : "অবস্থা: উইন্ডো স্বাভাবিক মোডে ফেরত এসেছে (Always On Top: OFF)");
}

void ClipboardGrabber::trigger_shortcut_action(const QString &action_id) {
    // Intentionally empty: ShortcutManager already ran executeAction before
    // emitting actionTriggered. Re-running it caused skip-by-2 on cycle
    // actions (format / section / subject / diagram template).
    Q_UNUSED(action_id);
}
