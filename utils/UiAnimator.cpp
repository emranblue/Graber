/**
 * UiAnimator implementation is split for smaller, token-friendly units:
 *   UiAnimator_Internal.h   — shared AnimState / clearState / naturalHeight
 *   UiAnimator_Visibility.cpp — stop + setVisibleSmooth
 *   UiAnimator_Window.cpp     — resize / fadeIn / showDialog / execDialog
 *   UiAnimator_Combo.cpp      — ComboPopupAnimator + enableComboPopupAnimation
 *   UiAnimator_Text.cpp       — setTextSmooth
 *
 * Public API remains in UiAnimator.h. This file is intentionally empty.
 */
