#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

#include "interfaces/IClipboardMonitor.h"
#include <QTimer>

class ClipboardMonitor : public IClipboardMonitor {
    Q_OBJECT

public:
    explicit ClipboardMonitor(QObject *parent = nullptr);
    ~ClipboardMonitor() override = default;

    void start(QClipboard::Mode mode, int intervalMs = 1000) override;
    void stop() override;
    bool isRunning() const override;

    void setMode(QClipboard::Mode mode) override;
    QClipboard::Mode mode() const override;

private slots:
    void checkClipboard();

private:
    QTimer *timer_;
    QClipboard::Mode mode_;
    bool is_running_;
    QString last_simplified_text_;
};

#endif // CLIPBOARDMONITOR_H
