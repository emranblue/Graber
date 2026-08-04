#include "ClipboardGrabber.h"
#include "ActionRegistry.h"
#include "MarkdownUtils.h"
#include "DiagramTemplates.h"
#include "utils/CrashGuard.h"
#include "utils/UiAnimator.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QTimer>
#include <QImage>

void ClipboardGrabber::start_monitoring() {
    if (ui_.subject_dropdown->currentIndex() == -1) {
        ui_.status_label->setText("অবস্থা: অনুগ্রহ করে প্রথমে একটি বিষয় নির্বাচন করুন!");
        return;
    }
    is_running_ = true;

    last_date_ = MarkdownUtils::restore_state_from_file(get_current_target_file());
    note_service_.updateTocInFile(get_current_target_file(), get_sections_from_ui());

    QClipboard::Mode mode = (ui_.mode_dropdown->currentIndex() == 0)
        ? QClipboard::Clipboard : QClipboard::Selection;
    clipboard_monitor_->start(mode, 1000);

    ui_.subject_dropdown->setEnabled(false);
    ui_.folder_dropdown->setEnabled(false);
    ui_.add_folder_button->setEnabled(false);
    ui_.add_subject_button->setEnabled(false);
    ui_.mode_dropdown->setEnabled(false);
    update_status_label();
    ActionRegistry::instance().updateBoundButtons();

    // Soft collapse panels + shrink window together (not only after panels finish).
    UiAnimator::setVisibleSmooth(
        {ui_.subject_card, ui_.capture_extra, ui_.heading_card, ui_.diagram_quick_row},
        false, UiAnimator::kPanelDurationMs,
        [this]() { fit_window_to_content(); });
    // Early window shrink so the frame eases down while panels are collapsing.
    QTimer::singleShot(UiAnimator::kPanelDurationMs / 3, this,
                       &ClipboardGrabber::fit_window_to_content);
}

void ClipboardGrabber::stop_monitoring() {
    is_running_ = false;
    clipboard_monitor_->stop();
    note_service_.updateTocInFile(get_current_target_file(), get_sections_from_ui());

    ui_.subject_dropdown->setEnabled(true);
    ui_.folder_dropdown->setEnabled(true);
    ui_.add_folder_button->setEnabled(true);
    ui_.add_subject_button->setEnabled(true);
    ui_.mode_dropdown->setEnabled(true);
    update_status_label();
    ActionRegistry::instance().updateBoundButtons();

    // Soft expand panels + grow window back to full shape.
    const bool show_diagram = diagram_panel_enabled_ && !is_running_;
    QList<QWidget *> to_show{ui_.subject_card, ui_.capture_extra, ui_.heading_card};
    if (show_diagram)
        to_show.append(ui_.diagram_quick_row);
    else if (ui_.diagram_quick_row)
        UiAnimator::setVisibleSmooth(ui_.diagram_quick_row, false);

    UiAnimator::setVisibleSmooth(
        to_show, true, UiAnimator::kPanelDurationMs,
        [this]() { fit_window_to_content(); });
    // Mid-wave window grow so the frame expands with the panels, not after a snap.
    QTimer::singleShot(UiAnimator::kPanelDurationMs / 3, this,
                       &ClipboardGrabber::fit_window_to_content);
}

void ClipboardGrabber::handle_text_captured(const QString &text) {
    // Never let exceptions escape into the Qt event loop.
    CrashGuard::safeCall([&]() {
        if (is_diagram_format_selected()) {
            handle_diagram_capture(text);
            return;
        }
        ui_.last_captured_label->setText("শেষ ক্যাপচার: " + text);
        write_to_file(text, ui_.section_dropdown->currentData().toString());
    }, QStringLiteral("handle_text_captured"));
}

void ClipboardGrabber::handle_diagram_capture(const QString &text) {
    if (text.trimmed().isEmpty()) return;

    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") return;

    diagram_nodes_.append(text);
    QString template_id = ui_.diagram_dropdown->currentData().toString();
    QString md = DiagramTemplates::buildFromNodes(template_id, diagram_nodes_);

    if (note_service_.upsertLiveDiagram(target_file, diagram_session_id_, md,
                                        selected_heading_slug_, last_date_)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        if (diagram_nodes_.size() == 1) {
            ui_.last_captured_label->setText("রুট নোড সেট হয়েছে: " + text);
        } else {
            ui_.last_captured_label->setText(
                QString("সাব নোড যুক্ত হয়েছে (#%1): %2")
                    .arg(diagram_nodes_.size() - 1).arg(text));
        }
    } else {
        ui_.last_captured_label->setText("ত্রুটি: ডায়াগ্রাম আপডেট করা যায়নি!");
    }
}

void ClipboardGrabber::start_new_diagram_session() {
    diagram_nodes_.clear();
    diagram_session_id_ = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
}

void ClipboardGrabber::handle_image_captured(const QImage &image) {
    Q_UNUSED(image);
}

void ClipboardGrabber::add_clipboard_image() {
    QImage image = QGuiApplication::clipboard()->image();
    if (image.isNull()) {
        ui_.last_captured_label->setText("ছবি যুক্ত করার ত্রুটি: ক্লিপবোর্ডে কোনো ছবি নেই!");
        return;
    }

    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") return;

    QDir images_dir(QFileInfo(target_file).dir().filePath("images"));
    if (!images_dir.exists()) images_dir.mkpath(".");

    QString filename = "img_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png";
    QString filepath = images_dir.filePath(filename);

    if (image.save(filepath, "PNG")) {
        note_service_.writeImageToNote(target_file, filename, last_date_);
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText("ছবি সফলভাবে যুক্ত করা হয়েছে: " + filename);
    } else {
        ui_.last_captured_label->setText("ছবি সংরক্ষণ করতে ব্যর্থ!");
    }
}

void ClipboardGrabber::insert_diagram() {
    start_new_diagram_session();
    ui_.last_captured_label->setText("নতুন ডায়াগ্রাম প্রস্তুত: পরবর্তী কপি রুট নোড হবে");
}

void ClipboardGrabber::write_to_file(const QString &processed_text, const QString &section) {
    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") return;

    QString out_label;
    int format_index = ui_.format_dropdown->currentIndex();
    bool ok = note_service_.writeToNote(target_file, processed_text, format_index,
                                        section, selected_heading_slug_, last_date_, out_label);
    if (!out_label.isEmpty())
        ui_.last_captured_label->setText(out_label);
    if (ok) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        if (format_index == 1)
            populate_headings_from_file();
    }
}

bool ClipboardGrabber::is_diagram_format_selected() const {
    return ui_.format_dropdown->currentText() == kDiagramFormatLabel;
}

void ClipboardGrabber::on_format_changed(int) {
    apply_diagram_format_lock();
}

void ClipboardGrabber::apply_diagram_format_lock() {
    bool diagramSelected = is_diagram_format_selected();
    ui_.diagram_dropdown->setEnabled(diagramSelected);
    if (diagramSelected)
        start_new_diagram_session();

    ActionRegistry::instance().updateBoundButtons();
    ui_.status_label->setText(diagramSelected
        ? "অবস্থা: ডায়াগ্রাম মোড চালু (প্রথম কপি রুট নোড, পরেরগুলো সাব নোড হবে)"
        : "অবস্থা: ডায়াগ্রাম মোড বন্ধ (ফরম্যাট আবার সক্রিয়)");
}

void ClipboardGrabber::apply_diagram_panel_visibility() {
    bool visible = diagram_panel_enabled_ && !is_running_;
    UiAnimator::setVisibleSmooth(
        ui_.diagram_quick_row, visible, UiAnimator::kPanelDurationMs,
        [this]() { fit_window_to_content(); });
    if (!visible && is_diagram_format_selected())
        ui_.format_dropdown->setCurrentIndex(0);
}
