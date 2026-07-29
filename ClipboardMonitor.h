#ifndef CLIPBOARDMONITOR_H
#define CLIPBOARDMONITOR_H

#include <QObject>
#include <QTimer>
#include <QClipboard>
#include <QImage>

class ClipboardMonitor : public QObject {
    Q_OBJECT

public:
    explicit ClipboardMonitor(QObject *parent = nullptr);

    void start(QClipboard::Mode mode, int intervalMs = 1000);
    void stop();
    bool isRunning() const;

    void setMode(QClipboard::Mode mode);
    QClipboard::Mode mode() const;

signals:
    void textCaptured(const QString &text);
    void imageCaptured(const QImage &image);

private slots:
    void checkClipboard();

private:
    QTimer *timer_;
    QClipboard::Mode mode_;
    bool is_running_;
    QString last_simplified_text_;
};

#endif // CLIPBOARDMONITOR_H
