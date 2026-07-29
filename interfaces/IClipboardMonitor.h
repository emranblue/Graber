#ifndef ICLIPBOARDMONITOR_H
#define ICLIPBOARDMONITOR_H

#include <QObject>
#include <QClipboard>
#include <QImage>
#include <QString>

class IClipboardMonitor : public QObject {
    Q_OBJECT
public:
    explicit IClipboardMonitor(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IClipboardMonitor() = default;

    virtual void start(QClipboard::Mode mode, int intervalMs = 1000) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual void setMode(QClipboard::Mode mode) = 0;
    virtual QClipboard::Mode mode() const = 0;

signals:
    void textCaptured(const QString &text);
    void imageCaptured(const QImage &image);
};

#endif // ICLIPBOARDMONITOR_H
