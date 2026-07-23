#include "clipboard_monitor.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QImage>

ClipboardMonitor::ClipboardMonitor(QObject *parent)
    : QObject(parent), last_simplified_text_(""), is_running_(false), check_interval_(1000) {
    
    clipboard_timer_ = new QTimer(this);
    clipboard_timer_->setInterval(check_interval_);
    connect(clipboard_timer_, &QTimer::timeout, this, &ClipboardMonitor::checkClipboard);
}

ClipboardMonitor::~ClipboardMonitor() {
}

void ClipboardMonitor::start() {
    if (!is_running_) {
        is_running_ = true;
        last_simplified_text_ = QGuiApplication::clipboard()->text().simplified();
        clipboard_timer_->start();
    }
}

void ClipboardMonitor::stop() {
    if (is_running_) {
        is_running_ = false;
        clipboard_timer_->stop();
    }
}

bool ClipboardMonitor::isRunning() const {
    return is_running_;
}

void ClipboardMonitor::checkClipboard() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString current_text = clipboard->text();
    QString simplified_text = current_text.simplified();
    
    if (!simplified_text.isEmpty() && simplified_text != last_simplified_text_) {
        last_simplified_text_ = simplified_text;
        emit newTextCaptured(simplified_text);
    }
}
