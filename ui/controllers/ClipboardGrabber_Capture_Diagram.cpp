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

void ClipboardGrabber::insert_diagram() {
    start_new_diagram_session();
    ui_.last_captured_label->setText("নতুন ডায়াগ্রাম প্রস্তুত: পরবর্তী কপি রুট নোড হবে");
}

bool ClipboardGrabber::is_diagram_format_selected() const {
    return ui_.format_dropdown->currentText() == kDiagramFormatLabel;
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

void ClipboardGrabber::refresh_active_diagram() {
    if (diagram_nodes_.isEmpty() || diagram_session_id_.isEmpty() || !is_diagram_format_selected())
        return;
    QString target_file = get_current_target_file();
    if (target_file == "নির্বাচিত নয়") return;

    QString template_id = ui_.diagram_dropdown->currentData().toString();
    if (template_id.isEmpty())
        template_id = QStringLiteral("flowchart");

    QString md = DiagramTemplates::buildFromNodes(template_id, diagram_nodes_);
    if (note_service_.upsertLiveDiagram(target_file, diagram_session_id_, md,
                                        selected_heading_slug_, last_date_)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText(
            QString("ডায়াগ্রাম টেমপ্লেট রিফ্রেশ হয়েছে: %1")
                .arg(ui_.diagram_dropdown->currentText()));
    }
}

void ClipboardGrabber::on_diagram_format_changed(int index) {
    Q_UNUSED(index);
    refresh_active_diagram();
}

void ClipboardGrabber::apply_diagram_panel_visibility() {
    bool visible = diagram_panel_enabled_ && !is_running_;
    UiAnimator::setVisibleSmooth(
        ui_.diagram_quick_row, visible, UiAnimator::kPanelDurationMs,
        [this]() { fit_window_to_content(); });
    if (!visible && is_diagram_format_selected())
        ui_.format_dropdown->setCurrentIndex(0);
}
