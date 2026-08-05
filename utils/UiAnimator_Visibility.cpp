#include "UiAnimator.h"
#include "UiAnimator_Internal.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QTimer>
#include <QLayout>
#include <memory>

using UiAnimatorInternal::clearState;
using UiAnimatorInternal::remember;
using UiAnimatorInternal::restoreConstraints;
using UiAnimatorInternal::finalizeHide;
using UiAnimatorInternal::naturalHeight;
using UiAnimatorInternal::states;

namespace UiAnimator {

void stop(QWidget *widget) {
    clearState(widget);
}

// ---------------------------------------------------------------------------
// Panel visibility (height collapse / expand)
// ---------------------------------------------------------------------------

void setVisibleSmooth(QWidget *widget, bool visible, int durationMs) {
    if (!widget)
        return;
    setVisibleSmooth(QList<QWidget *>{widget}, visible, durationMs, {}, false);
}

void setVisibleSmooth(QWidget *widget, bool visible, int durationMs,
                      std::function<void()> onFinished) {
    if (!widget) {
        if (onFinished)
            onFinished();
        return;
    }
    setVisibleSmooth(QList<QWidget *>{widget}, visible, durationMs, std::move(onFinished), false);
}

void setVisibleSmooth(const QList<QWidget *> &widgets, bool visible,
                      int durationMs, std::function<void()> onFinished,
                      bool stagger) {
    if (widgets.isEmpty()) {
        if (onFinished)
            onFinished();
        return;
    }

    // Filter nulls and widgets that are already in the target state.
    QList<QWidget *> work;
    work.reserve(widgets.size());
    for (QWidget *w : widgets) {
        if (!w)
            continue;
        const bool needs =
            (w->isVisible() != visible) ||
            (visible && w->maximumHeight() == 0) ||
            (!visible && w->isVisible() && w->height() > 0);
        if (needs)
            work.append(w);
    }
    if (work.isEmpty()) {
        if (onFinished)
            onFinished();
        return;
    }

    // Shared counter so onFinished runs exactly once after the last animation.
    auto remaining = std::make_shared<int>(work.size());
    auto finishedOnce = std::make_shared<std::function<void()>>(std::move(onFinished));

    auto onOneDone = [remaining, finishedOnce]() {
        if (--(*remaining) == 0 && *finishedOnce) {
            (*finishedOnce)();
            *finishedOnce = nullptr;
        }
    };

    for (int i = 0; i < work.size(); ++i) {
        QWidget *w = work[i];
        const int delay = (stagger && work.size() > 1) ? i * kStaggerMs : 0;

        auto startAnim = [w, visible, durationMs, onOneDone]() {
            if (!w) {
                onOneDone();
                return;
            }
            clearState(w);
            restoreConstraints(w);

            auto *group = new QParallelAnimationGroup(w);

            if (visible) {
                if (!w->isVisible()) {
                    w->setMaximumHeight(0);
                    w->setVisible(true);
                    if (w->parentWidget() && w->parentWidget()->layout()) {
                        w->parentWidget()->layout()->invalidate();
                        w->parentWidget()->layout()->activate();
                    }
                }
                const int endH = naturalHeight(w);
                w->setMaximumHeight(0);

                auto *hAnim = new QPropertyAnimation(w, "maximumHeight", group);
                hAnim->setDuration(durationMs);
                hAnim->setEasingCurve(QEasingCurve::OutCubic);
                hAnim->setStartValue(0);
                hAnim->setEndValue(qMax(endH, 1));
                group->addAnimation(hAnim);
            } else {
                const int startH = w->isVisible()
                    ? (w->height() > 0 ? w->height() : naturalHeight(w))
                    : 0;
                if (!w->isVisible() || startH <= 0) {
                    delete group;
                    onOneDone();
                    return;
                }
                w->setMaximumHeight(startH);

                auto *hAnim = new QPropertyAnimation(w, "maximumHeight", group);
                hAnim->setDuration(durationMs);
                hAnim->setEasingCurve(QEasingCurve::InCubic);
                hAnim->setStartValue(startH);
                hAnim->setEndValue(0);
                group->addAnimation(hAnim);
            }

            remember(w, group);

            QObject::connect(group, &QParallelAnimationGroup::finished, w,
                             [w, visible, onOneDone, group]() {
                                 if (w) {
                                     if (visible) {
                                         w->setVisible(true);
                                         restoreConstraints(w);
                                     } else {
                                         finalizeHide(w);
                                     }
                                     states().remove(w);
                                 }
                                 onOneDone();
                                 group->deleteLater();
                             });

            group->start();
        };

        if (delay > 0)
            QTimer::singleShot(delay, w, startAnim);
        else
            startAnim();
    }
}

} // namespace UiAnimator
