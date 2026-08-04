#ifndef GRABER_UIANIMATOR_H
#define GRABER_UIANIMATOR_H

#include <QWidget>
#include <QList>
#include <functional>

/**
 * Soft, non-shaky UI transitions for panel show/hide and window resize.
 *
 * - Panel hide/show: maximumHeight collapse/expand animation (InOutCubic)
 * - Window resize: animated geometry (OutCubic) so the frame never jumps
 *
 * All animations are cancel-safe: starting a new one on the same widget
 * stops the previous group first.
 */
namespace UiAnimator {

/** Default durations (ms). Soft enough to feel intentional, short enough to stay responsive. */
constexpr int kPanelDurationMs = 240;
constexpr int kWindowDurationMs = 260;

/**
 * Smoothly show or hide a widget (collapse/expand height).
 * When hiding finishes, the widget is setVisible(false) and height limits cleared.
 * When showing starts, the widget is made visible at opacity 0 / height 0 first.
 */
void setVisibleSmooth(QWidget *widget, bool visible, int durationMs = kPanelDurationMs);

/** Single widget + finish callback (forwards to the list overload). */
void setVisibleSmooth(QWidget *widget, bool visible, int durationMs,
                      std::function<void()> onFinished);

/** Show/hide several widgets together (same timing, shared optional finish callback). */
void setVisibleSmooth(const QList<QWidget *> &widgets, bool visible,
                      int durationMs = kPanelDurationMs,
                      std::function<void()> onFinished = {});

/**
 * Animate the window geometry to the given size (keeps top-left fixed).
 * Clamps against the screen available geometry so it never overshoots.
 */
void resizeWindowSmooth(QWidget *window, const QSize &targetSize,
                        int durationMs = kWindowDurationMs);

/** Stop any in-flight animation owned by this helper on the given widget. */
void stop(QWidget *widget);

} // namespace UiAnimator

#endif // GRABER_UIANIMATOR_H
