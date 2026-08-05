#include "UiAnimator.h"
#include "UiAnimator_Internal.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>

using UiAnimatorInternal::clearState;
using UiAnimatorInternal::remember;
using UiAnimatorInternal::states;

namespace UiAnimator {

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

} // namespace UiAnimator
