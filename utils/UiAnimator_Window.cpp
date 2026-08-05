#include "UiAnimator.h"
#include "UiAnimator_Internal.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <functional>

using UiAnimatorInternal::clearState;
using UiAnimatorInternal::remember;
using UiAnimatorInternal::states;

namespace UiAnimator {

void resizeWindowSmooth(QWidget *window, const QSize &targetSize, int durationMs,
                        std::function<void()> onFinished) {
    if (!window) {
        if (onFinished)
            onFinished();
        return;
    }

    clearState(window);

    QRect geo = window->geometry();
    int tw = targetSize.width();
    int th = targetSize.height();

    if (QScreen *scr = window->screen()
                           ? window->screen()
                           : QGuiApplication::primaryScreen()) {
        const QRect avail = scr->availableGeometry();
        tw = qMin(tw, avail.width() - 16);
        th = qMin(th, avail.height() - 16);
        if (geo.x() + tw > avail.right())
            geo.moveLeft(qMax(avail.left(), avail.right() - tw + 1));
        if (geo.y() + th > avail.bottom())
            geo.moveTop(qMax(avail.top(), avail.bottom() - th + 1));
    }

    // Use target size directly — do NOT clamp to current minimumHeight here.
    // Callers that temporarily lower minimumSize rely on this so the window
    // can grow/shrink smoothly instead of snapping to a raised min size.
    const QRect endRect(geo.x(), geo.y(), qMax(tw, 1), qMax(th, 1));

    if (window->geometry() == endRect) {
        if (onFinished)
            onFinished();
        return;
    }

    auto *anim = new QPropertyAnimation(window, "geometry", window);
    anim->setDuration(durationMs);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(window->geometry());
    anim->setEndValue(endRect);

    remember(window, anim);
    QObject::connect(anim, &QPropertyAnimation::finished, window,
                     [window, onFinished]() {
                         states().remove(window);
                         if (onFinished)
                             onFinished();
                     });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ---------------------------------------------------------------------------
// Window / dialog fade-in
// ---------------------------------------------------------------------------

void fadeInWindow(QWidget *window, int durationMs) {
    if (!window)
        return;

    // Window opacity is only reliable on top-level windows with a backing store.
    if (!window->isWindow()) {
        window->show();
        return;
    }

    clearState(window);
    window->setWindowOpacity(0.0);
    window->show();

    auto *anim = new QPropertyAnimation(window, "windowOpacity", window);
    anim->setDuration(durationMs);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);

    remember(window, anim);
    QObject::connect(anim, &QPropertyAnimation::finished, window, [window]() {
        if (window)
            window->setWindowOpacity(1.0);
        states().remove(window);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void fadeOutWindow(QWidget *window, int durationMs,
                   std::function<void()> onFinished) {
    if (!window) {
        if (onFinished)
            onFinished();
        return;
    }

    // Non-top-level or platforms without opacity: finish immediately.
    if (!window->isWindow()) {
        if (onFinished)
            onFinished();
        return;
    }

    clearState(window);

    // Guard against concurrent close storms.
    if (window->property("_uia_fading_out").toBool()) {
        if (onFinished)
            onFinished();
        return;
    }
    window->setProperty("_uia_fading_out", true);

    const qreal start = window->windowOpacity() > 0.01
                            ? window->windowOpacity()
                            : 1.0;
    window->setWindowOpacity(start);

    auto *anim = new QPropertyAnimation(window, "windowOpacity", window);
    anim->setDuration(qMax(1, durationMs));
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->setStartValue(start);
    anim->setEndValue(0.0);

    remember(window, anim);
    QObject::connect(anim, &QPropertyAnimation::finished, window,
                     [window, onFinished]() {
                         if (window) {
                             window->setProperty("_uia_fading_out", false);
                             states().remove(window);
                         }
                         if (onFinished)
                             onFinished();
                     });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}


} // namespace UiAnimator
