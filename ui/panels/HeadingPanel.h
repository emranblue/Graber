#ifndef HEADINGPANEL_H
#define HEADINGPANEL_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>

/**
 * Target-heading control card:
 * select / append / inject / shift / delete.
 */
class HeadingPanel : public QFrame {
    Q_OBJECT
public:
    explicit HeadingPanel(QWidget *parent = nullptr);

    QPushButton *selectHeadingButton() const { return select_heading_button_; }
    QPushButton *appendButton() const { return append_to_heading_button_; }
    QPushButton *injectButton() const { return inject_heading_button_; }
    QPushButton *shiftButton() const { return shift_heading_button_; }
    QPushButton *deleteButton() const { return delete_heading_button_; }

    QLabel *headingLabel() const { return heading_label_; }

    void setSelectedTitle(const QString &title);
    void clearSelection();

private:
    QLabel *heading_label_ = nullptr;
    QPushButton *select_heading_button_ = nullptr;
    QPushButton *append_to_heading_button_ = nullptr;
    QPushButton *inject_heading_button_ = nullptr;
    QPushButton *shift_heading_button_ = nullptr;
    QPushButton *delete_heading_button_ = nullptr;
};

#endif // HEADINGPANEL_H
