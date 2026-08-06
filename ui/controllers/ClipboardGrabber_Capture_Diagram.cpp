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

// Live / dynamic diagram capture
// --------------------------------
// Every clipboard event is processed immediately:
//   1. Append the new text to the session node list
//   2. Rebuild the full diagram from the complete list
//      (1st node = root, rest = sub-nodes)
//   3. Upsert the live diagram block in the note file
// There is no deferred / buffered write – the note is updated
// on every single capture so the user sees the diagram grow in real time.
void ClipboardGrabber::handle_diagram_capture(const QString &text) {
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) return;

    QString target_file = get_current_target_file();
    if (target_file == QStringLiteral("নির্বাচিত নয়")) return;

    // Ensure we have a live session
    if (diagram_session_id_.isEmpty())
        start_new_diagram_session();

    // Dynamic addition – immediately extend the node list
    diagram_nodes_.append(trimmed);

    const QString template_id = ui_.diagram_dropdown->currentData().toString();
    const QString md = DiagramTemplates::buildFromNodes(template_id, diagram_nodes_);

    if (note_service_.upsertLiveDiagram(target_file, diagram_session_id_, md,
                                        selected_heading_slug_, last_date_)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());

        if (diagram_nodes_.size() == 1) {
            ui_.last_captured_label->setText(
                QStringLiteral("রুট নোড সেট হয়েছে: %1").arg(trimmed));
        } else {
            ui_.last_captured_label->setText(
                QStringLiteral("সাব নোড যুক্ত হয়েছে (#%1): %2")
                    .arg(diagram_nodes_.size() - 1)
                    .arg(trimmed));
        }
    } else {
        // Roll back the node we just added so the list stays consistent
        diagram_nodes_.removeLast();
        ui_.last_captured_label->setText(
            QStringLiteral("ত্রুটি: ডায়াগ্রাম আপডেট করা যায়নি!"));
    }
}

void ClipboardGrabber::start_new_diagram_session() {
    diagram_nodes_.clear();
    diagram_session_id_ = QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
}

void ClipboardGrabber::insert_diagram() {
    start_new_diagram_session();
    ui_.last_captured_label->setText(
        QStringLiteral("নতুন ডায়াগ্রাম প্রস্তুত: পরবর্তী কপি রুট নোড হবে"));
}

bool ClipboardGrabber::is_diagram_format_selected() const {
    return ui_.format_dropdown->currentText() == kDiagramFormatLabel;
}

void ClipboardGrabber::apply_diagram_format_lock() {
    const bool diagramSelected = is_diagram_format_selected();
    ui_.diagram_dropdown->setEnabled(diagramSelected);
    if (diagramSelected)
        start_new_diagram_session();

    ActionRegistry::instance().updateBoundButtons();
    ui_.status_label->setText(diagramSelected
        ? QStringLiteral("অবস্থা: ডায়াগ্রাম মোড চালু (প্রথম কপি রুট নোড, পরেরগুলো সাব নোড হবে)")
        : QStringLiteral("অবস্থা: ডায়াগ্রাম মোড বন্ধ (ফরম্যাট আবার সক্রিয়)"));
}

void ClipboardGrabber::refresh_active_diagram() {
    if (diagram_nodes_.isEmpty() || diagram_session_id_.isEmpty() || !is_diagram_format_selected())
        return;

    QString target_file = get_current_target_file();
    if (target_file == QStringLiteral("নির্বাচিত নয়")) return;

    QString template_id = ui_.diagram_dropdown->currentData().toString();
    if (template_id.isEmpty())
        template_id = QStringLiteral("flowchart");

    const QString md = DiagramTemplates::buildFromNodes(template_id, diagram_nodes_);
    if (note_service_.upsertLiveDiagram(target_file, diagram_session_id_, md,
                                        selected_heading_slug_, last_date_)) {
        note_service_.updateTocInFile(target_file, get_sections_from_ui());
        ui_.last_captured_label->setText(
            QStringLiteral("ডায়াগ্রাম টেমপ্লেট রিফ্রেশ হয়েছে: %1")
                .arg(ui_.diagram_dropdown->currentText()));
    }
}

void ClipboardGrabber::on_diagram_format_changed(int index) {
    Q_UNUSED(index);
    refresh_active_diagram();
}

void ClipboardGrabber::apply_diagram_panel_visibility() {
    const bool visible = diagram_panel_enabled_ && !is_running_;
    UiAnimator::setVisibleSmooth(
        ui_.diagram_quick_row, visible, UiAnimator::kPanelDurationMs,
        [this]() { fit_window_to_content(); });
    if (!visible && is_diagram_format_selected())
        ui_.format_dropdown->setCurrentIndex(0);
}
