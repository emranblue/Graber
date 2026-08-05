#ifndef GRABER_UIANIMATOR_INTERNAL_H
#define GRABER_UIANIMATOR_INTERNAL_H

/**
 * Shared bookkeeping for UiAnimator implementation units.
 * Not part of the public API — only included from UiAnimator_*.cpp.
 */
#include <QWidget>
#include <QAbstractAnimation>
#include <QHash>
#include <QPointer>
#include <QLayout>

namespace UiAnimatorInternal {

struct AnimState {
    QPointer<QAbstractAnimation> group;
};

inline QHash<QWidget *, AnimState> &states() {
    static QHash<QWidget *, AnimState> s;
    return s;
}

inline void clearState(QWidget *w) {
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

inline void remember(QWidget *w, QAbstractAnimation *group) {
    if (!w || !group)
        return;
    AnimState st;
    st.group = group;
    states().insert(w, st);
}

inline void restoreConstraints(QWidget *w) {
    if (!w)
        return;
    w->setMaximumHeight(QWIDGETSIZE_MAX);
    w->setMinimumHeight(0);
}

inline void finalizeHide(QWidget *w) {
    if (!w)
        return;
    w->setVisible(false);
    restoreConstraints(w);
}

inline int naturalHeight(QWidget *w) {
    if (!w)
        return 0;
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

} // namespace UiAnimatorInternal

#endif // GRABER_UIANIMATOR_INTERNAL_H
