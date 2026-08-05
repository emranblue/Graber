#include "ControlsBar.h"
#include "Utils.h"
#include <QSize>
#include <QHBoxLayout>

ControlsBar::ControlsBar(QWidget *parent) : QWidget(parent) {
    setObjectName("controlsBar");

    start_button_ = new QPushButton("শুরু (Start)", this);
    start_button_->setObjectName("primaryActionButton");
    start_button_->setStyleSheet(
        "QPushButton { background-color: #34c759; color: white; border-radius: 4px; padding: 6px; }"
        "QPushButton:hover { background-color: #34c759; opacity: 0.9; }");
    start_button_->setIcon(get_feather_icon(QChar(0xe9a8)));
    start_button_->setIconSize(QSize(16, 16));

    stop_button_ = new QPushButton("থামুন (Stop)", this);
    stop_button_->setObjectName("primaryActionButton");
    stop_button_->setEnabled(false);
    stop_button_->setStyleSheet(
        "QPushButton { background-color: #ff3b30; color: white; border-radius: 4px; padding: 6px; }"
        "QPushButton:hover { background-color: #d70015; }");
    stop_button_->setIcon(get_feather_icon(QChar(0xe9e4)));
    stop_button_->setIconSize(QSize(16, 16));

    wizards_button_ = new QPushButton("টুলস", this);
    wizards_button_->setToolTip(
        QStringLiteral("কনফিগ ও টুলস খুলুন (Open config tools)"));
    wizards_button_->setStyleSheet(
        "QPushButton { background-color: #af52de; color: white; border-radius: 4px; padding: 6px; }"
        "QPushButton:hover { background-color: #bf5af2; }");
    wizards_button_->setIcon(get_feather_icon(QChar(0xe9e9)));
    wizards_button_->setIconSize(QSize(16, 16));

    settings_button_ = new QPushButton("সেটিংস", this);
    settings_button_->setObjectName("secondaryButton");
    settings_button_->setStyleSheet(
        "QPushButton { background-color: #8e8e93; color: white; border-radius: 4px; padding: 6px; }"
        "QPushButton:hover { background-color: #1d1d1f; }");
    settings_button_->setIcon(get_feather_icon(QChar(0xe9db)));
    settings_button_->setIconSize(QSize(16, 16));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(6);
    layout->addWidget(start_button_, 1);
    layout->addWidget(stop_button_, 1);
    layout->addWidget(wizards_button_);
    layout->addWidget(settings_button_);
}
