#include "ControlsBar.h"
#include "Utils.h"
#include <QHBoxLayout>

ControlsBar::ControlsBar(QWidget *parent) : QWidget(parent) {
    setObjectName("controlsBar");

    start_button_ = new QPushButton("শুরু (Start)", this);
    start_button_->setObjectName("primaryActionButton");
    start_button_->setStyleSheet(
        "QPushButton { background-color: #44bd32; color: white; border-radius: 4px; padding: 6px; }"
        "QPushButton:hover { background-color: #44bd32; opacity: 0.9; }");
    start_button_->setIcon(get_feather_icon(QChar(0xe9a8)));

    stop_button_ = new QPushButton("থামুন (Stop)", this);
    stop_button_->setObjectName("primaryActionButton");
    stop_button_->setEnabled(false);
    stop_button_->setStyleSheet(
        "QPushButton { background-color: #e84118; color: white; border-radius: 4px; padding: 6px; }"
        "QPushButton:hover { background-color: #c23616; }");
    stop_button_->setIcon(get_feather_icon(QChar(0xe9e4)));

    wizards_button_ = new QPushButton("উইজার্ড ও টুলস", this);
    wizards_button_->setStyleSheet(
        "QPushButton { background-color: #8e44ad; color: white; border-radius: 4px; padding: 6px; }"
        "QPushButton:hover { background-color: #9b59b6; }");
    wizards_button_->setIcon(get_feather_icon(QChar(0xe9b8)));

    settings_button_ = new QPushButton("সেটিংস", this);
    settings_button_->setObjectName("secondaryButton");
    settings_button_->setStyleSheet(
        "QPushButton { background-color: #718093; color: white; border-radius: 4px; padding: 6px; }"
        "QPushButton:hover { background-color: #2f3640; }");
    settings_button_->setIcon(get_feather_icon(QChar(0xe9db)));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(6);
    layout->addWidget(start_button_, 1);
    layout->addWidget(stop_button_, 1);
    layout->addWidget(wizards_button_);
    layout->addWidget(settings_button_);
}
