#ifndef CONTROLSBAR_H
#define CONTROLSBAR_H

#include <QWidget>
#include <QPushButton>

/**
 * Bottom primary-action strip: Start / Stop / Wizards / Settings.
 */
class ControlsBar : public QWidget {
    Q_OBJECT
public:
    explicit ControlsBar(QWidget *parent = nullptr);

    QPushButton *startButton() const { return start_button_; }
    QPushButton *stopButton() const { return stop_button_; }
    QPushButton *wizardsButton() const { return wizards_button_; }
    QPushButton *settingsButton() const { return settings_button_; }

private:
    QPushButton *start_button_ = nullptr;
    QPushButton *stop_button_ = nullptr;
    QPushButton *wizards_button_ = nullptr;
    QPushButton *settings_button_ = nullptr;
};

#endif // CONTROLSBAR_H
