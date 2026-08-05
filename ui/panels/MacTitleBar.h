#ifndef MACTITLEBAR_H
#define MACTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPoint>

/**
 * Soft Mac-style title bar: traffic-light controls + centered title + drag.
 * Used with Qt::FramelessWindowHint on the host window.
 */
class MacTitleBar : public QWidget {
    Q_OBJECT
public:
    explicit MacTitleBar(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    QString title() const;

signals:
    void closeRequested();
    void minimizeRequested();
    void maximizeRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPushButton *makeTrafficLight(const QString &objectName, const QString &baseColor,
                                  const QString &hoverColor);
    QWidget *windowHost() const;

    QPushButton *close_btn_ = nullptr;
    QPushButton *min_btn_ = nullptr;
    QPushButton *max_btn_ = nullptr;
    QLabel *title_label_ = nullptr;
    QLabel *icon_label_ = nullptr;

    bool dragging_ = false;
    QPoint drag_offset_;
};

#endif // MACTITLEBAR_H
