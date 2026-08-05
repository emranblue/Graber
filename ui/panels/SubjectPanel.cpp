#include "SubjectPanel.h"
#include "Utils.h"
#include <QSize>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
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

SubjectPanel::SubjectPanel(QWidget *parent) : QFrame(parent) {
    setObjectName("card");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(8);
    layout->addWidget(sectionHeader(QChar(0xe9d0), QColor("#487eb0"),
                                    "ফোল্ডার ও বিষয়ের তালিকা (Folder & Subject Navigation)"));

    folder_dropdown_ = new QComboBox(this);
    folder_dropdown_->setIconSize(QSize(18, 18));

    subject_dropdown_ = new QComboBox(this);
    subject_dropdown_->setIconSize(QSize(18, 18));

    add_folder_button_ = new QPushButton("নতুন ফোল্ডার", this);
    add_folder_button_->setStyleSheet(
        "QPushButton { background-color: #ff9500; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #e68600; }");
    add_folder_button_->setIcon(get_feather_icon(QChar(0xe9c9)));
    add_folder_button_->setIconSize(QSize(16, 16));

    toggle_subject_button_ = new QPushButton("বিষয় পরিবর্তন", this);
    toggle_subject_button_->setStyleSheet(
        "QPushButton { background-color: #af52de; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #bf5af2; }");
    toggle_subject_button_->setIcon(get_feather_icon(QChar(0xe9d0)));
    toggle_subject_button_->setIconSize(QSize(16, 16));

    add_subject_button_ = new QPushButton("নতুন বিষয়", this);
    add_subject_button_->setStyleSheet(
        "QPushButton { background-color: #34c759; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #34c759; opacity: 0.9; }");
    add_subject_button_->setIcon(get_feather_icon(QChar(0xe9c9)));
    add_subject_button_->setIconSize(QSize(16, 16));

    open_file_button_ = new QPushButton("নোট খুলুন", this);
    open_file_button_->setStyleSheet(
        "QPushButton { background-color: #007aff; color: white; border-radius: 4px; padding: 5px; }"
        "QPushButton:hover { background-color: #0066d6; }");
    open_file_button_->setEnabled(false);
    open_file_button_->setIcon(get_feather_icon(QChar(0xe966)));
    open_file_button_->setIconSize(QSize(16, 16));

    // Folder row
    auto *folder_layout = new QHBoxLayout();
    folder_layout->setSpacing(8);
    QLabel *lbl_folder = new QLabel("ফোল্ডার:");
    lbl_folder->setFixedWidth(48);
    folder_layout->addWidget(lbl_folder);
    folder_layout->addWidget(folder_dropdown_, 1);
    folder_layout->addWidget(add_folder_button_);
    layout->addLayout(folder_layout);

    // Subject row
    auto *file_layout = new QHBoxLayout();
    file_layout->setSpacing(8);
    QLabel *lbl_subject = new QLabel("বিষয়:");
    lbl_subject->setFixedWidth(48);
    file_layout->addWidget(lbl_subject);
    file_layout->addWidget(subject_dropdown_, 1);
    file_layout->addWidget(toggle_subject_button_);
    file_layout->addWidget(add_subject_button_);
    file_layout->addWidget(open_file_button_);
    layout->addLayout(file_layout);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setXOffset(0);
    shadow->setYOffset(6);
    shadow->setColor(QColor(0, 0, 0, 28));
    setGraphicsEffect(shadow);
}
