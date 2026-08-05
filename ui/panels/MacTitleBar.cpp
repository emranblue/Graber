#include "MacTitleBar.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEvent>
#include <QApplication>
#include <QWindow>
#include <QStyle>
#include "Utils.h"
#include <QtGlobal>

MacTitleBar::MacTitleBar(QWidget *parent) : QWidget(parent) {
    setObjectName("macTitleBar");
    setFixedHeight(40);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setStyleSheet(
        "QWidget#macTitleBar {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #fafafa, stop:1 #f0f0f2);"
        "  border-bottom: 1px solid rgba(0,0,0,0.08);"
        "}"
        "QLabel#macTitleLabel {"
        "  color: #1d1d1f; font-size: 13px; font-weight: 600;"
        "  background: transparent; letter-spacing: 0.2px;"
        "}"
        "QPushButton#macClose, QPushButton#macMin, QPushButton#macMax {"
        "  border: none; border-radius: 7px; min-width: 12px; max-width: 12px;"
        "  min-height: 12px; max-height: 12px; padding: 0; margin: 0;"
        "}"
        "QPushButton#macClose { background-color: #ff5f57; }"
        "QPushButton#macClose:hover { background-color: #ff3b30; }"
        "QPushButton#macMin { background-color: #febc2e; }"
        "QPushButton#macMin:hover { background-color: #f5a623; }"
        "QPushButton#macMax { background-color: #28c840; }"
        "QPushButton#macMax:hover { background-color: #34c759; }"
        "QPushButton#macClose:pressed, QPushButton#macMin:pressed,"
        "QPushButton#macMax:pressed { opacity: 0.85; }"
    );

    close_btn_ = makeTrafficLight("macClose", "#ff5f57", "#ff3b30");
    min_btn_ = makeTrafficLight("macMin", "#febc2e", "#f5a623");
    max_btn_ = makeTrafficLight("macMax", "#28c840", "#34c759");

    close_btn_->setToolTip("Close");
    min_btn_->setToolTip("Minimize");
    max_btn_->setToolTip("Maximize");

    title_label_ = new QLabel("Clipboard Graber", this);
    title_label_->setObjectName("macTitleLabel");
    title_label_->setAlignment(Qt::AlignCenter);
    title_label_->setAttribute(Qt::WA_TransparentForMouseEvents);

    icon_label_ = new QLabel(this);
    icon_label_->setFixedSize(20, 20);
    icon_label_->setScaledContents(true);
    icon_label_->setPixmap(get_app_icon().pixmap(20, 20));
    icon_label_->setAttribute(Qt::WA_TransparentForMouseEvents);
    icon_label_->setToolTip(QStringLiteral("Clipboard Graber"));

    auto *left = new QHBoxLayout();
    left->setContentsMargins(0, 0, 0, 0);
    left->setSpacing(8);
    left->addWidget(close_btn_);
    left->addWidget(min_btn_);
    left->addWidget(max_btn_);
    left->addStretch(1);

    // Centered title with small app mark (Mac-like toolbar identity)
    auto *center = new QHBoxLayout();
    center->setContentsMargins(0, 0, 0, 0);
    center->setSpacing(8);
    center->addStretch(1);
    center->addWidget(icon_label_);
    center->addWidget(title_label_);
    center->addStretch(1);

    auto *right = new QHBoxLayout();
    right->setContentsMargins(0, 0, 0, 0);
    right->addStretch(1);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 0, 14, 0);
    layout->setSpacing(0);
    layout->addLayout(left, 1);
    layout->addLayout(center, 2);
    layout->addLayout(right, 1);

    connect(close_btn_, &QPushButton::clicked, this, &MacTitleBar::closeRequested);
    connect(min_btn_, &QPushButton::clicked, this, &MacTitleBar::minimizeRequested);
    connect(max_btn_, &QPushButton::clicked, this, &MacTitleBar::maximizeRequested);

    // Hover expand slightly for touch targets without changing visual size much
    close_btn_->installEventFilter(this);
    min_btn_->installEventFilter(this);
    max_btn_->installEventFilter(this);
}

QPushButton *MacTitleBar::makeTrafficLight(const QString &objectName,
                                           const QString &baseColor,
                                           const QString &hoverColor) {
    Q_UNUSED(baseColor);
    Q_UNUSED(hoverColor);
    auto *btn = new QPushButton(this);
    btn->setObjectName(objectName);
    btn->setCursor(Qt::ArrowCursor);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setFlat(true);
    return btn;
}

void MacTitleBar::setTitle(const QString &title) {
    if (title_label_)
        title_label_->setText(title.isEmpty() ? QStringLiteral("Clipboard Graber") : title);
}

QString MacTitleBar::title() const {
    return title_label_ ? title_label_->text() : QString();
}

QWidget *MacTitleBar::windowHost() const {
    QWidget *w = window();
    return w ? w : parentWidget();
}

void MacTitleBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Don't start drag from traffic lights (they handle their own clicks).
        QWidget *child = childAt(event->pos());
        if (child == close_btn_ || child == min_btn_ || child == max_btn_) {
            QWidget::mousePressEvent(event);
            return;
        }
        dragging_ = true;
        if (QWidget *host = windowHost()) {
            #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            drag_offset_ = event->globalPosition().toPoint() - host->frameGeometry().topLeft();
#else
            drag_offset_ = event->globalPos() - host->frameGeometry().topLeft();
#endif
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void MacTitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (dragging_ && (event->buttons() & Qt::LeftButton)) {
        if (QWidget *host = windowHost()) {
            if (host->isMaximized()) {
                // Leave maximized before dragging.
                host->showNormal();
                drag_offset_ = QPoint(host->width() / 2, height() / 2);
            }
            #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            host->move(event->globalPosition().toPoint() - drag_offset_);
#else
            host->move(event->globalPos() - drag_offset_);
#endif
        }
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void MacTitleBar::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        dragging_ = false;
    QWidget::mouseReleaseEvent(event);
}

void MacTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit maximizeRequested();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

bool MacTitleBar::eventFilter(QObject *watched, QEvent *event) {
    Q_UNUSED(watched);
    Q_UNUSED(event);
    return QWidget::eventFilter(watched, event);
}
