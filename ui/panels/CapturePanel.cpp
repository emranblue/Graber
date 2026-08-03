#include "CapturePanel.h"
#include "Utils.h"
#include "DiagramTemplates.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QColor>

namespace {
QWidget *sectionHeader(const QChar &glyphCode, const QColor &accent, const QString &text) {
    QWidget *row = new QWidget();
    auto *l = new QHBoxLayout(row);
    l->setContentsMargins(0, 0, 0, 3);
    l->setSpacing(6);

    QLabel *badge = new QLabel();
    badge->setFixedSize(22, 22);
    badge->setAlignment(Qt::AlignCenter);
    badge->setStyleSheet(QString("background-color: %1; border-radius: 11px;").arg(accent.lighter(175).name()));
    badge->setPixmap(get_feather_icon(glyphCode, accent.darker(110), 14).pixmap(14, 14));

    QLabel *title = new QLabel(text);
    title->setStyleSheet("font-weight: 700; font-size: 13px; color: #192a56;");

    l->addWidget(badge);
    l->addWidget(title, 1);
    return row;
}
} // namespace

CapturePanel::CapturePanel(QWidget *parent) : QFrame(parent) {
    setObjectName("card");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(8);
    layout->addWidget(sectionHeader(QChar(0xe9a8), QColor("#8c7ae6"),
                                    "ক্যাপচার ও ইনপুট কনফিগারেশন (Capture Configuration)"));

    // Format + Mode row (always visible while capturing)
    format_dropdown_ = new QComboBox(this);
    format_dropdown_->addItem("বুলেট পয়েন্ট (Point)");
    format_dropdown_->addItem("প্রধান শিরোনাম (Heading - Red)");
    format_dropdown_->addItem("উপ-শিরোনাম (Subheading - Blue)");
    format_dropdown_->addItem("মাইন্ড ম্যাপ (Timeline Mind Map)");
    format_dropdown_->addItem("প্যারাগ্রাফ (Paragraph)");
    format_dropdown_->addItem(kDiagramFormatLabel);

    mode_label_ = new QLabel("মোড:", this);
    mode_dropdown_ = new QComboBox(this);
    mode_dropdown_->addItem("কপি মোড (Ctrl+C)");
    mode_dropdown_->addItem("সিলেক্ট মোড");

    auto *options_layout = new QHBoxLayout();
    options_layout->setSpacing(8);
    options_layout->addWidget(new QLabel("ফরম্যাট:"));
    options_layout->addWidget(format_dropdown_, 1);
    options_layout->addWidget(mode_label_);
    options_layout->addWidget(mode_dropdown_, 1);
    layout->addLayout(options_layout);

    // Diagram quick-insert row
    diagram_dropdown_ = new QComboBox(this);
    diagram_dropdown_->setEnabled(false);
    for (const auto &tpl : DiagramTemplates::list()) {
        diagram_dropdown_->addItem(tpl.second, tpl.first);
    }

    insert_diagram_button_ = new QPushButton("নতুন ডায়াগ্রাম (New Diagram)", this);
    insert_diagram_button_->setStyleSheet(
        "QPushButton { background-color: #00a8ff; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #0097e6; }");
    insert_diagram_button_->setEnabled(false);
    insert_diagram_button_->setIcon(get_feather_icon(QChar(0xe992)));

    diagram_quick_row_ = new QWidget(this);
    auto *diagram_layout = new QHBoxLayout(diagram_quick_row_);
    diagram_layout->setContentsMargins(0, 0, 0, 0);
    diagram_layout->setSpacing(8);
    diagram_layout->addWidget(diagram_dropdown_, 1);
    diagram_layout->addWidget(insert_diagram_button_);
    layout->addWidget(diagram_quick_row_);

    // Extra controls (section + image) — hidden during active capture
    capture_extra_ = new QWidget(this);
    auto *extra_layout = new QVBoxLayout(capture_extra_);
    extra_layout->setContentsMargins(0, 0, 0, 0);
    extra_layout->setSpacing(10);

    section_label_ = new QLabel("বিভাগ (Section):", this);
    section_dropdown_ = new QComboBox(this);

    add_section_button_ = new QPushButton("নতুন বিভাগ", this);
    add_section_button_->setStyleSheet(
        "QPushButton { background-color: #44bd32; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");
    add_section_button_->setIcon(get_feather_icon(QChar(0xe9c9)));

    auto *section_layout = new QHBoxLayout();
    section_layout->setSpacing(8);
    section_layout->addWidget(section_label_);
    section_layout->addWidget(section_dropdown_, 1);
    section_layout->addWidget(add_section_button_);
    extra_layout->addLayout(section_layout);

    add_image_button_ = new QPushButton("ছবি যুক্ত করুন (Add Image)", this);
    add_image_button_->setIcon(get_feather_icon(QChar(0xe978)));

    auto *image_layout = new QHBoxLayout();
    image_layout->addWidget(add_image_button_, 1);
    extra_layout->addLayout(image_layout);

    layout->addWidget(capture_extra_);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(16);
    shadow->setXOffset(0);
    shadow->setYOffset(3);
    shadow->setColor(QColor(25, 42, 86, 30));
    setGraphicsEffect(shadow);
}

bool CapturePanel::isDiagramFormatSelected() const {
    return format_dropdown_->currentText() == kDiagramFormatLabel;
}

void CapturePanel::setDiagramDropdownEnabled(bool enabled) {
    diagram_dropdown_->setEnabled(enabled);
}

void CapturePanel::setExtraVisible(bool visible) {
    capture_extra_->setVisible(visible);
}

void CapturePanel::setDiagramRowVisible(bool visible) {
    diagram_quick_row_->setVisible(visible);
}
