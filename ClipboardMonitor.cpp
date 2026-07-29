#include "ClipboardMonitor.h"
#include <QGuiApplication>

ClipboardMonitor::ClipboardMonitor(QObject *parent)
    : QObject(parent), mode_(QClipboard::Clipboard), is_running_(false) {
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &ClipboardMonitor::checkClipboard);
}

void ClipboardMonitor::start(QClipboard::Mode mode, int intervalMs) {
    mode_ = mode;
    last_simplified_text_ = QGuiApplication::clipboard()->text(mode_).simplified();
    timer_->start(intervalMs);
    is_running_ = true;
}

void ClipboardMonitor::stop() {
    timer_->stop();
    is_running_ = false;
}

bool ClipboardMonitor::isRunning() const {
    return is_running_;
}

void ClipboardMonitor::setMode(QClipboard::Mode mode) {
    mode_ = mode;
}

QClipboard::Mode ClipboardMonitor::mode() const {
    return mode_;
}

void ClipboardMonitor::checkClipboard() {
    if (!is_running_) return;

    QClipboard *clipboard = QGuiApplication::clipboard();
    QString current_text = clipboard->text(mode_);
    QString simplified_text = current_text.simplified();

    if (!simplified_text.isEmpty() && simplified_text != last_simplified_text_) {
        last_simplified_text_ = simplified_text;
        emit textCaptured(simplified_text);
    }
}
