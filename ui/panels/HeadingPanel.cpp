#include "HeadingPanel.h"
#include "Utils.h"
#include <QSize>
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

HeadingPanel::HeadingPanel(QWidget *parent) : QFrame(parent) {
    setObjectName("card");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(8);
    layout->addWidget(sectionHeader(QChar(0xe90a), QColor("#e68600"),
                                    "টার্গেট শিরোনাম নিয়ন্ত্রণ (Target Heading Control)"));

    heading_label_ = new QLabel("শিরোনাম (Heading):", this);

    select_heading_button_ = new QPushButton("(শেষে নতুন করে যোগ করুন / Append to End)", this);
    select_heading_button_->setObjectName("select_heading_button");
    select_heading_button_->setStyleSheet(
        "QPushButton { background-color: white; color: black; border: 1px solid #dcdde1; "
        "text-align: left; padding: 7px; font-weight: normal; border-radius: 4px; }"
        "QPushButton:hover { background-color: #f5f6fa; }"
        "QPushButton:disabled { background-color: #dcdde1; color: #7f8c8d; }");
    select_heading_button_->setEnabled(false);
    select_heading_button_->setIcon(get_feather_icon(QChar(0xe90a), QColor("#2f3640")));
    select_heading_button_->setIconSize(QSize(16, 16));

    append_to_heading_button_ = new QPushButton("যুক্ত করুন", this);
    append_to_heading_button_->setStyleSheet(
        "QPushButton { background-color: #ff9500; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #e68600; }");
    append_to_heading_button_->setEnabled(false);
    append_to_heading_button_->setIcon(get_feather_icon(QChar(0xe963)));
    append_to_heading_button_->setIconSize(QSize(16, 16));

    inject_heading_button_ = new QPushButton("ইনজেক্ট করুন", this);
    inject_heading_button_->setStyleSheet(
        "QPushButton { background-color: #5856d6; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #7d7aff; }");
    inject_heading_button_->setEnabled(false);
    inject_heading_button_->setIcon(get_feather_icon(QChar(0xe992)));
    inject_heading_button_->setIconSize(QSize(16, 16));

    shift_heading_button_ = new QPushButton("স্থানান্তর", this);
    shift_heading_button_->setStyleSheet(
        "QPushButton { background-color: #007aff; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #0066d6; }");
    shift_heading_button_->setEnabled(false);
    shift_heading_button_->setIcon(get_feather_icon(QChar(0xe9bc)));
    shift_heading_button_->setIconSize(QSize(16, 16));

    delete_heading_button_ = new QPushButton("মুছে ফেলুন", this);
    delete_heading_button_->setStyleSheet(
        "QPushButton { background-color: #ff3b30; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #d70015; }");
    delete_heading_button_->setEnabled(false);
    delete_heading_button_->setIcon(get_feather_icon(QChar(0xe9f6)));
    delete_heading_button_->setIconSize(QSize(16, 16));

    auto *heading_layout = new QHBoxLayout();
    heading_layout->setSpacing(8);
    heading_layout->addWidget(heading_label_);
    heading_layout->addWidget(select_heading_button_, 1);
    layout->addLayout(heading_layout);

    auto *actions_layout = new QHBoxLayout();
    actions_layout->setSpacing(8);
    actions_layout->addWidget(append_to_heading_button_);
    actions_layout->addWidget(inject_heading_button_);
    actions_layout->addWidget(shift_heading_button_);
    actions_layout->addWidget(delete_heading_button_);
    layout->addLayout(actions_layout);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setXOffset(0);
    shadow->setYOffset(6);
    shadow->setColor(QColor(0, 0, 0, 28));
    setGraphicsEffect(shadow);
}

void HeadingPanel::setSelectedTitle(const QString &title) {
    if (title.isEmpty()) {
        select_heading_button_->setText("(শেষে নতুন করে যোগ করুন / Append to End)");
    } else {
        select_heading_button_->setText(title);
    }
}

void HeadingPanel::clearSelection() {
    setSelectedTitle(QString());
}
