#ifndef CLIPBOARD_MONITOR_H
#define CLIPBOARD_MONITOR_H

#include <QObject>
#include <QTimer>
#include <QString>

class ClipboardMonitor : public QObject {
    Q_OBJECT

public:
    explicit ClipboardMonitor(QObject *parent = nullptr);
    ~ClipboardMonitor();
    
    void start();
    void stop();
    bool isRunning() const;
    
signals:
    void newTextCaptured(const QString &text);
    void newImageCaptured(const QString &imagePath);
    
private slots:
    void checkClipboard();
    
private:
    QTimer *clipboard_timer_;
    QString last_simplified_text_;
    bool is_running_;
    int check_interval_;
};

#endif // CLIPBOARD_MONITOR_H
