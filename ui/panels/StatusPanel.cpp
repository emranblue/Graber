#include "StatusPanel.h"
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QColor>
#include <QStyle>

StatusPanel::StatusPanel(QWidget *parent) : QFrame(parent) {
    setObjectName("heroCard");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    status_label_ = new QLabel("অবস্থা: বন্ধ", this);
    status_label_->setObjectName("status_label");
    status_label_->setAlignment(Qt::AlignCenter);

    last_captured_label_ = new QLabel("শেষ ক্যাপচার: (কিছুই না)", this);
    last_captured_label_->setObjectName("last_captured_label");
    last_captured_label_->setAlignment(Qt::AlignCenter);
    last_captured_label_->setWordWrap(true);
    last_captured_label_->setMinimumHeight(46);

    layout->addWidget(status_label_);
    layout->addWidget(last_captured_label_);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setXOffset(0);
    shadow->setYOffset(6);
    shadow->setColor(QColor(0, 0, 0, 28));
    setGraphicsEffect(shadow);
}

void StatusPanel::setRunning(bool running) {
    status_label_->setProperty("running", running);
    status_label_->style()->unpolish(status_label_);
    status_label_->style()->polish(status_label_);
}

void StatusPanel::setStatusText(const QString &text) {
    status_label_->setText(text);
}

void StatusPanel::setLastCaptured(const QString &text) {
    last_captured_label_->setText(text);
}
