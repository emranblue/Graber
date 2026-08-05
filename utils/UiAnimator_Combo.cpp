#include "UiAnimator.h"
#include "UiAnimator_Internal.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QAbstractItemView>
#include <QEvent>
#include <QTimer>
#include <QHash>
#include <QPointer>

using UiAnimatorInternal::clearState;
using UiAnimatorInternal::remember;
using UiAnimatorInternal::states;

namespace {

class ComboPopupAnimator : public QObject {
public:
    explicit ComboPopupAnimator(QComboBox *combo, int durationMs, QObject *parent = nullptr)
        : QObject(parent), combo_(combo), durationMs_(durationMs) {
        if (!combo_)
            return;
        // View is created lazily; install after the event loop starts.
        QTimer::singleShot(0, this, [this]() { installOnView(); });
        // Re-check when the user opens it the first time (view may appear later).
        combo_->installEventFilter(this);
        connect(combo_, &QComboBox::destroyed, this, &QObject::deleteLater);
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (combo_ && obj == combo_ && event->type() == QEvent::MouseButtonPress) {
            // Ensure the view filter is attached before the popup shows.
            installOnView();
        }

        QWidget *popup = popupWindow();
        const bool isPopup = (popup && obj == popup) || (view_ && obj == view_);
        if (!isPopup)
            return QObject::eventFilter(obj, event);

        if (event->type() == QEvent::Show || event->type() == QEvent::ShowToParent) {
            // Defer one tick so Qt finishes placing the popup geometry first.
            QTimer::singleShot(0, this, [this]() { animateOpen(); });
        }
        return QObject::eventFilter(obj, event);
    }

private:
    QWidget *popupWindow() const {
        if (!view_)
            return nullptr;
        QWidget *w = view_->window();
        if (w && w != combo_ && w != view_->parentWidget())
            return w;
        // Some styles parent the view under a QFrame that is the popup.
        QWidget *p = view_->parentWidget();
        while (p && p != combo_) {
            if (p->isWindow())
                return p;
            p = p->parentWidget();
        }
        return view_->isWindow() ? static_cast<QWidget *>(view_) : nullptr;
    }

    void installOnView() {
        if (!combo_)
            return;
        QAbstractItemView *v = combo_->view();
        if (!v)
            return;
        if (v != view_) {
            if (view_)
                view_->removeEventFilter(this);
            view_ = v;
            view_->installEventFilter(this);
        }
        QWidget *popup = popupWindow();
        if (popup && popup != tracked_popup_) {
            if (tracked_popup_)
                tracked_popup_->removeEventFilter(this);
            tracked_popup_ = popup;
            tracked_popup_->installEventFilter(this);
        }
    }

    void animateOpen() {
        installOnView();
        QWidget *popup = popupWindow();
        if (!popup || !popup->isVisible())
            return;

        // Cancel any in-flight fade on this popup.
        clearState(popup);

        // Opacity fade is safe: does not change hit-testing layout size.
        const qreal startOpacity = 0.0;
        popup->setWindowOpacity(startOpacity);

        // Optional tiny rise (4px) — pure position, no size clamp.
        const QPoint endPos = popup->pos();
        const QPoint startPos(endPos.x(), endPos.y() + 6);
        popup->move(startPos);

        auto *group = new QParallelAnimationGroup(popup);

        auto *fade = new QPropertyAnimation(popup, "windowOpacity", group);
        fade->setDuration(durationMs_);
        fade->setEasingCurve(QEasingCurve::OutCubic);
        fade->setStartValue(startOpacity);
        fade->setEndValue(1.0);
        group->addAnimation(fade);

        auto *slide = new QPropertyAnimation(popup, "pos", group);
        slide->setDuration(durationMs_);
        slide->setEasingCurve(QEasingCurve::OutCubic);
        slide->setStartValue(startPos);
        slide->setEndValue(endPos);
        group->addAnimation(slide);

        remember(popup, group);
        QObject::connect(group, &QParallelAnimationGroup::finished, popup,
                         [popup, group]() {
                             if (popup)
                                 popup->setWindowOpacity(1.0);
                             states().remove(popup);
                             group->deleteLater();
                         });
        group->start();
    }

    QPointer<QComboBox> combo_;
    QPointer<QAbstractItemView> view_;
    QPointer<QWidget> tracked_popup_;
    int durationMs_ = 180;
};

QHash<QComboBox *, ComboPopupAnimator *> &comboAnimators() {
    static QHash<QComboBox *, ComboPopupAnimator *> m;
    return m;
}


} // namespace

namespace UiAnimator {

void enableComboPopupAnimation(QComboBox *combo, int durationMs) {
    if (!combo)
        return;
    auto &map = comboAnimators();
    if (map.contains(combo) && map.value(combo))
        return;

    auto *anim = new ComboPopupAnimator(combo, durationMs, combo);
    map.insert(combo, anim);
    QObject::connect(combo, &QObject::destroyed, [combo]() {
        comboAnimators().remove(combo);
    });
}

} // namespace UiAnimator
