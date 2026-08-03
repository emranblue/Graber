#ifndef STATUSPANEL_H
#define STATUSPANEL_H

#include <QFrame>
#include <QLabel>

/**
 * Hero status card: live monitoring state + last-captured preview.
 * Pure presentation — no business logic.
 */
class StatusPanel : public QFrame {
    Q_OBJECT
public:
    explicit StatusPanel(QWidget *parent = nullptr);

    void setRunning(bool running);
    void setStatusText(const QString &text);
    void setLastCaptured(const QString &text);

    QLabel *statusLabel() const { return status_label_; }
    QLabel *lastCapturedLabel() const { return last_captured_label_; }

private:
    QLabel *status_label_ = nullptr;
    QLabel *last_captured_label_ = nullptr;
};

#endif // STATUSPANEL_H
