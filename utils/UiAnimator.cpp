#include "UiAnimator.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QScreen>
#include <QGuiApplication>
#include <QHash>
#include <QPointer>
#include <QLayout>

namespace {

// Track active groups so a new animation can cancel the previous one on the
// same widget without leaving height constraints stuck.
struct AnimState {
    QPointer<QParallelAnimationGroup> group;
};

QHash<QWidget *, AnimState> &states() {
    static QHash<QWidget *, AnimState> s;
    return s;
}

void clearState(QWidget *w) {
    if (!w)
        return;
    auto it = states().find(w);
    if (it == states().end())
        return;
    if (it->group) {
        it->group->stop();
        it->group->deleteLater();
    }
    states().erase(it);
}

void restoreConstraints(QWidget *w) {
    if (!w)
        return;
    w->setMaximumHeight(QWIDGETSIZE_MAX);
    w->setMinimumHeight(0);
}

void finalizeHide(QWidget *w) {
    if (!w)
        return;
    w->setVisible(false);
    restoreConstraints(w);
}

int naturalHeight(QWidget *w) {
    if (!w)
        return 0;
    int h = qMax(w->sizeHint().height(), w->minimumSizeHint().height());
    if (h <= 0)
        h = w->height() > 0 ? w->height() : 40;
    return h;
}

} // namespace

namespace UiAnimator {

void stop(QWidget *widget) {
    clearState(widget);
}

void setVisibleSmooth(QWidget *widget, bool visible, int durationMs) {
    if (!widget)
        return;
    setVisibleSmooth(QList<QWidget *>{widget}, visible, durationMs, {});
}

void setVisibleSmooth(QWidget *widget, bool visible, int durationMs,
                      std::function<void()> onFinished) {
    if (!widget) {
        if (onFinished)
            onFinished();
        return;
    }
    setVisibleSmooth(QList<QWidget *>{widget}, visible, durationMs, std::move(onFinished));
}

void setVisibleSmooth(const QList<QWidget *> &widgets, bool visible,
                      int durationMs, std::function<void()> onFinished) {
    if (widgets.isEmpty()) {
        if (onFinished)
            onFinished();
        return;
    }

    bool anyNeedsWork = false;
    for (QWidget *w : widgets) {
        if (!w)
            continue;
        if (w->isVisible() != visible) {
            anyNeedsWork = true;
            break;
        }
        if (visible && w->maximumHeight() == 0)
            anyNeedsWork = true;
    }
    if (!anyNeedsWork) {
        if (onFinished)
            onFinished();
        return;
    }

    auto *master = new QParallelAnimationGroup();

    for (QWidget *w : widgets) {
        if (!w)
            continue;

        clearState(w);

        // Height-only animation: avoids replacing QGraphicsDropShadowEffect
        // that cards already install, so shadows stay intact.
        restoreConstraints(w);

        if (visible) {
            if (!w->isVisible()) {
                // Make visible but collapsed so layout can measure, then expand.
                w->setMaximumHeight(0);
                w->setVisible(true);
                if (w->parentWidget() && w->parentWidget()->layout()) {
                    w->parentWidget()->layout()->invalidate();
                    w->parentWidget()->layout()->activate();
                }
            }
            const int endH = naturalHeight(w);
            w->setMaximumHeight(0);

            auto *hAnim = new QPropertyAnimation(w, "maximumHeight", master);
            hAnim->setDuration(durationMs);
            hAnim->setEasingCurve(QEasingCurve::InOutCubic);
            hAnim->setStartValue(0);
            hAnim->setEndValue(endH);
        } else {
            const int startH = w->isVisible()
                ? (w->height() > 0 ? w->height() : naturalHeight(w))
                : 0;
            if (!w->isVisible() || startH <= 0) {
                // Already gone — nothing to animate.
                continue;
            }
            w->setMaximumHeight(startH);

            auto *hAnim = new QPropertyAnimation(w, "maximumHeight", master);
            hAnim->setDuration(durationMs);
            hAnim->setEasingCurve(QEasingCurve::InOutCubic);
            hAnim->setStartValue(startH);
            hAnim->setEndValue(0);
        }

        AnimState st;
        st.group = master;
        states().insert(w, st);
    }

    if (master->animationCount() == 0) {
        // Nothing animated (already in target state).
        delete master;
        for (QWidget *w : widgets) {
            if (!w)
                continue;
            if (visible) {
                w->setVisible(true);
                restoreConstraints(w);
            } else {
                finalizeHide(w);
            }
        }
        if (onFinished)
            onFinished();
        return;
    }

    QWidget *owner = nullptr;
    for (QWidget *w : widgets) {
        if (w) {
            owner = w;
            break;
        }
    }
    if (owner)
        master->setParent(owner);

    QObject::connect(master, &QParallelAnimationGroup::finished, master,
                     [widgets, visible, onFinished, master]() {
                         for (QWidget *w : widgets) {
                             if (!w)
                                 continue;
                             if (visible) {
                                 w->setVisible(true);
                                 restoreConstraints(w);
                             } else {
                                 finalizeHide(w);
                             }
                             states().remove(w);
                         }
                         if (onFinished)
                             onFinished();
                         master->deleteLater();
                     });

    master->start();
}

void resizeWindowSmooth(QWidget *window, const QSize &targetSize, int durationMs) {
    if (!window)
        return;

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

    const QRect endRect(geo.x(), geo.y(),
                        qMax(tw, window->minimumWidth()),
                        qMax(th, window->minimumHeight()));

    if (window->geometry() == endRect)
        return;

    auto *anim = new QPropertyAnimation(window, "geometry", window);
    anim->setDuration(durationMs);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(window->geometry());
    anim->setEndValue(endRect);

    auto *group = new QParallelAnimationGroup(window);
    group->addAnimation(anim);

    AnimState st;
    st.group = group;
    states().insert(window, st);

    QObject::connect(group, &QParallelAnimationGroup::finished, group,
                     [window, group]() {
                         states().remove(window);
                         group->deleteLater();
                     });

    group->start();
}

} // namespace UiAnimator
