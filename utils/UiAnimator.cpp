#include "UiAnimator.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>
#include <QScreen>
#include <QGuiApplication>
#include <QHash>
#include <QPointer>
#include <QLayout>
#include <QGraphicsOpacityEffect>
#include <QAbstractItemView>
#include <QEvent>
#include <QTimer>
#include <QLabel>
#include <QVariantAnimation>
#include <QApplication>
#include <memory>

namespace {

// ---------------------------------------------------------------------------
// Active animation bookkeeping (cancel-safe)
// ---------------------------------------------------------------------------

struct AnimState {
    QPointer<QAbstractAnimation> group;
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

void remember(QWidget *w, QAbstractAnimation *group) {
    if (!w || !group)
        return;
    AnimState st;
    st.group = group;
    states().insert(w, st);
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
    // Prefer layout-driven size when available so animated end height matches
    // the real resting size after expand.
    int h = 0;
    if (w->layout()) {
        h = w->layout()->sizeHint().height();
        h = qMax(h, w->layout()->minimumSize().height());
    }
    h = qMax(h, w->sizeHint().height());
    h = qMax(h, w->minimumSizeHint().height());
    if (h <= 0)
        h = w->height() > 0 ? w->height() : 40;
    return h;
}

// ComboBox popup height animation was removed: clamping maximumHeight on the
// Qt popup view/container made dropdowns unclickable / collapse to 0px.
// Native QComboBox popups stay fully interactive.

} // namespace

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

// ---------------------------------------------------------------------------
// Window resize
// ---------------------------------------------------------------------------

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

    remember(window, anim);
    QObject::connect(anim, &QPropertyAnimation::finished, window, [window]() {
        states().remove(window);
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

void showDialogSmooth(QWidget *dialog, int durationMs) {
    if (!dialog)
        return;

    clearState(dialog);

    // Start slightly lower and transparent, then rise + fade in.
    const QPoint orig = dialog->pos();
    const bool wasVisible = dialog->isVisible();

    if (dialog->isWindow()) {
        dialog->setWindowOpacity(0.0);
        if (!wasVisible) {
            // Offset down a few pixels for the rise effect.
            dialog->move(orig.x(), orig.y() + 12);
        }
        dialog->show();

        auto *group = new QParallelAnimationGroup(dialog);

        auto *fade = new QPropertyAnimation(dialog, "windowOpacity", group);
        fade->setDuration(durationMs);
        fade->setEasingCurve(QEasingCurve::OutCubic);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        group->addAnimation(fade);

        if (!wasVisible) {
            auto *slide = new QPropertyAnimation(dialog, "pos", group);
            slide->setDuration(durationMs);
            slide->setEasingCurve(QEasingCurve::OutCubic);
            slide->setStartValue(QPoint(orig.x(), orig.y() + 12));
            slide->setEndValue(orig);
            group->addAnimation(slide);
        }

        remember(dialog, group);
        QObject::connect(group, &QParallelAnimationGroup::finished, dialog,
                         [dialog, group]() {
                             if (dialog)
                                 dialog->setWindowOpacity(1.0);
                             states().remove(dialog);
                             group->deleteLater();
                         });
        group->start();
    } else {
        dialog->show();
    }
}

int execDialogSmooth(QDialog *dialog, int durationMs) {
    if (!dialog)
        return QDialog::Rejected;

    // Prepare opacity/position before the nested event loop.
    const QPoint orig = dialog->pos().isNull()
        ? QPoint() // will be placed by Qt when shown
        : dialog->pos();
    Q_UNUSED(orig);

    dialog->setWindowOpacity(0.0);
    dialog->setAttribute(Qt::WA_DontShowOnScreen, false);

    // Show first so geometry is valid, then animate opacity.
    // We can't easily animate before exec() blocks, so kick off the fade
    // via a zero-timer after the dialog is about to enter its event loop.
    QTimer::singleShot(0, dialog, [dialog, durationMs]() {
        if (!dialog)
            return;
        clearState(dialog);

        auto *fade = new QPropertyAnimation(dialog, "windowOpacity", dialog);
        fade->setDuration(durationMs);
        fade->setEasingCurve(QEasingCurve::OutCubic);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);

        remember(dialog, fade);
        QObject::connect(fade, &QPropertyAnimation::finished, dialog, [dialog]() {
            if (dialog)
                dialog->setWindowOpacity(1.0);
            states().remove(dialog);
        });
        fade->start(QAbstractAnimation::DeleteWhenStopped);
    });

    const int result = dialog->exec();
    // Restore full opacity for next open (in case dialog is reused).
    dialog->setWindowOpacity(1.0);
    return result;
}

// ---------------------------------------------------------------------------
// ComboBox popup — intentionally a no-op
// ---------------------------------------------------------------------------

void enableComboPopupAnimation(QComboBox *combo, int /*durationMs*/) {
    // Kept for API compatibility. Animating QComboBox popups via maximumHeight
    // breaks click handling on the list; leave native popup behavior alone.
    Q_UNUSED(combo);
}

// ---------------------------------------------------------------------------
// Label text cross-fade
// ---------------------------------------------------------------------------

void setTextSmooth(QWidget *label, const QString &text, int durationMs) {
    if (!label)
        return;

    // Only animate QLabel (or anything with a "text" property).
    if (!label->property("text").isValid() && !qobject_cast<QLabel *>(label)) {
        if (auto *lbl = qobject_cast<QLabel *>(label))
            lbl->setText(text);
        return;
    }

    // Don't steal an existing graphics effect (e.g. drop shadow on cards).
    if (label->graphicsEffect()) {
        if (auto *lbl = qobject_cast<QLabel *>(label))
            lbl->setText(text);
        else
            label->setProperty("text", text);
        return;
    }

    clearState(label);

    auto *effect = new QGraphicsOpacityEffect(label);
    effect->setOpacity(1.0);
    label->setGraphicsEffect(effect);

    auto *seq = new QSequentialAnimationGroup(label);

    auto *fadeOut = new QPropertyAnimation(effect, "opacity", seq);
    fadeOut->setDuration(durationMs / 2);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    seq->addAnimation(fadeOut);

    // Swap text at the midpoint.
    QObject::connect(fadeOut, &QPropertyAnimation::finished, label, [label, text]() {
        if (!label)
            return;
        if (auto *lbl = qobject_cast<QLabel *>(label))
            lbl->setText(text);
        else
            label->setProperty("text", text);
    });

    auto *fadeIn = new QPropertyAnimation(effect, "opacity", seq);
    fadeIn->setDuration(durationMs / 2);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    seq->addAnimation(fadeIn);

    remember(label, seq);
    QObject::connect(seq, &QSequentialAnimationGroup::finished, label,
                     [label, effect, seq]() {
                         if (label) {
                             label->setGraphicsEffect(nullptr);
                             states().remove(label);
                         }
                         if (effect)
                             effect->deleteLater();
                         seq->deleteLater();
                     });
    seq->start();
}

} // namespace UiAnimator
