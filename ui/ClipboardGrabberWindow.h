#ifndef CLIPBOARDGRABBERWINDOW_H
#define CLIPBOARDGRABBERWINDOW_H

#include <QWidget>
#include <memory>

class ClipboardGrabberUI;

class ClipboardGrabberWindow : public QWidget {
    Q_OBJECT

public:
    explicit ClipboardGrabberWindow(QWidget *parent = nullptr);
    ~ClipboardGrabberWindow() override;

    void setMonitoringActive(bool active);
    void fitWindowToContent();

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onInsertDiagramClicked();

private:
    void setupConnections();

    std::unique_ptr<ClipboardGrabberUI> ui;
    bool is_monitoring_ = false;
};

#endif // CLIPBOARDGRABBERWINDOW_H
