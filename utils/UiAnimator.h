#ifndef GRABER_UIANIMATOR_H
#define GRABER_UIANIMATOR_H

#include <QWidget>
#include <QList>
#include <QDialog>
#include <QComboBox>
#include <functional>

/**
 * Soft, cancel-safe UI transitions for every interactive surface:
 *
 * - Panel hide/show: height collapse/expand + optional opacity (InOutCubic / OutCubic)
 * - Staggered multi-panel waves so cards don't snap in lockstep
 * - Window resize: animated geometry (OutCubic), never jumps
 * - Dialog open/close: fade + slight rise / fall
 * - ComboBox popup: height expand/collapse on the drop-down view
 * - Status / label cross-fade helpers
 *
 * Starting a new animation on the same widget cancels the previous group
 * first so height/opacity constraints never stick.
 */
namespace UiAnimator {

/** Default durations (ms). Soft enough to feel intentional, short enough to stay responsive. */
constexpr int kPanelDurationMs   = 280;
constexpr int kWindowDurationMs  = 300;
constexpr int kDialogDurationMs  = 220;
constexpr int kPopupDurationMs   = 180;
constexpr int kFadeDurationMs    = 160;
constexpr int kStaggerMs         = 35;   // delay between successive panels in a wave

/**
 * Smoothly show or hide a widget (collapse/expand height).
 * When hiding finishes, the widget is setVisible(false) and height limits cleared.
 * When showing starts, the widget is made visible at height 0 first.
 */
void setVisibleSmooth(QWidget *widget, bool visible, int durationMs = kPanelDurationMs);

/** Single widget + finish callback (forwards to the list overload). */
void setVisibleSmooth(QWidget *widget, bool visible, int durationMs,
                      std::function<void()> onFinished);

/**
 * Show/hide several widgets together.
 * When stagger is true (default), each card starts a few ms after the previous
 * for a soft cascade; onFinished still fires once when the last one finishes.
 */
void setVisibleSmooth(const QList<QWidget *> &widgets, bool visible,
                      int durationMs = kPanelDurationMs,
                      std::function<void()> onFinished = {},
                      bool stagger = true);

/**
 * Animate the window geometry to the given size (keeps top-left fixed).
 * Clamps against the screen available geometry so it never overshoots.
 * Optional onFinished runs after the resize settles (or immediately if no-op).
 */
void resizeWindowSmooth(QWidget *window, const QSize &targetSize,
                        int durationMs = kWindowDurationMs,
                        std::function<void()> onFinished = {});

/**
 * Fade a top-level window in from transparent (e.g. main window first show).
 * Safe no-op if the platform does not support window opacity.
 */
void fadeInWindow(QWidget *window, int durationMs = kDialogDurationMs);

/**
 * Open a modal dialog with fade + slight upward slide, then exec().
 * Returns the dialog's result code (Accepted / Rejected / …).
 * Falls back to plain exec() if animation cannot start.
 */
int execDialogSmooth(QDialog *dialog, int durationMs = kDialogDurationMs);

/**
 * Non-modal: show a dialog/window with the same fade + rise animation.
 */
void showDialogSmooth(QWidget *dialog, int durationMs = kDialogDurationMs);

/**
 * Soft open for QComboBox popups: opacity fade (+ tiny slide) on the popup
 * window only. Does NOT clamp maximumHeight (that broke click selection).
 * Call once after the combo is created (idempotent).
 */
void enableComboPopupAnimation(QComboBox *combo, int durationMs = kPopupDurationMs);

/**
 * Soft cross-fade of a label's text (opacity dip → change text → fade up).
 * If the label has a graphics effect already, falls back to instant setText.
 */
void setTextSmooth(QWidget *label, const QString &text, int durationMs = kFadeDurationMs);

/** Stop any in-flight animation owned by this helper on the given widget. */
void stop(QWidget *widget);

} // namespace UiAnimator

#endif // GRABER_UIANIMATOR_H
