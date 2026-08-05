#include "UiAnimator.h"
#include "UiAnimator_Internal.h"

#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QEasingCurve>
#include <QGraphicsOpacityEffect>
#include <QLabel>

using UiAnimatorInternal::clearState;
using UiAnimatorInternal::remember;
using UiAnimatorInternal::states;

namespace UiAnimator {

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
