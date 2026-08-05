#include "GlobalHotkeyListener.h"

#include <QApplication>
#include <QGuiApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <QSocketNotifier>
#endif

// ============================================================================
// GlobalHotkeyListener
// ============================================================================

#ifdef Q_OS_WIN
GlobalHotkeyListener::NativeEventFilter::NativeEventFilter(GlobalHotkeyListener *parent)
    : parent_(parent) {}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool GlobalHotkeyListener::NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) {
#else
bool GlobalHotkeyListener::NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
#endif
    Q_UNUSED(result);
    if (eventType == "windows_generic_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY) {
            int hotkeyId = static_cast<int>(msg->wParam);
            if (parent_->registered_hotkeys_.contains(hotkeyId)) {
                QString actionId = parent_->registered_hotkeys_[hotkeyId].actionId;
                emit parent_->globalHotkeyPressed(actionId);
                return true;
            }
        }
    }
    return false;
}
#endif

#ifdef Q_OS_LINUX
namespace {
    // XGrabKey() treats Num Lock / Caps Lock / Scroll Lock as regular
    // modifiers, so a hotkey grabbed only with (e.g.) ControlMask|ShiftMask
    // would silently stop firing the moment Num Lock is toggled on. Grabbing
    // every combination of the "lock" modifiers alongside the real ones
    // makes the hotkey fire regardless of lock-key state.
    const unsigned int kLockMaskCombos[] = { 0, LockMask, Mod2Mask, LockMask | Mod2Mask };
}
#endif

GlobalHotkeyListener::GlobalHotkeyListener(QObject *parent)
    : QObject(parent), is_supported_(true) {
#ifdef Q_OS_WIN
    next_hotkey_id_ = 1;
    native_filter_ = new NativeEventFilter(this);
    qApp->installNativeEventFilter(native_filter_);
#endif

#ifdef Q_OS_LINUX
    x11_display_ = XOpenDisplay(nullptr);
    x11_notifier_ = nullptr;
    x11_root_ = 0;

    if (x11_display_) {
        Display *display = static_cast<Display *>(x11_display_);
        x11_root_ = DefaultRootWindow(display);

        // Integrate the X11 connection's file descriptor into Qt's event
        // loop so we can watch for grabbed hotkeys without blocking or
        // needing a separate thread.
        int fd = ConnectionNumber(display);
        x11_notifier_ = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        connect(x11_notifier_, &QSocketNotifier::activated, this, &GlobalHotkeyListener::on_x11_activity);

        is_supported_ = true;
    } else {
        // No usable X11 connection (e.g. a pure Wayland session without
        // XWayland). Global hotkeys aren't available in that case; the app
        // still works fine, shortcuts just require the window to be focused.
        is_supported_ = false;
    }
#endif

#ifdef Q_OS_MAC
    is_supported_ = false; // Fallback for macOS (would need Cocoa support)
#endif
}

GlobalHotkeyListener::~GlobalHotkeyListener() {
    unregisterAllHotkeys();
#ifdef Q_OS_WIN
    if (native_filter_) {
        qApp->removeNativeEventFilter(native_filter_);
        delete native_filter_;
    }
#endif

#ifdef Q_OS_LINUX
    if (x11_display_) {
        XCloseDisplay(static_cast<Display *>(x11_display_));
        x11_display_ = nullptr;
    }
#endif
}

void GlobalHotkeyListener::registerGlobalHotkey(const QKeySequence &keySeq, const QString &actionId) {
    if (keySeq.isEmpty() || actionId.isEmpty()) return;

#ifdef Q_OS_WIN
    const int key = keySeq[0].toCombined();
    UINT modifiers = 0;
    if (key & Qt::CTRL)  modifiers |= MOD_CONTROL;
    if (key & Qt::ALT)   modifiers |= MOD_ALT;
    if (key & Qt::SHIFT) modifiers |= MOD_SHIFT;
    if (key & Qt::META)  modifiers |= MOD_WIN;

    const int actualKey = key & 0xFFFF;
    // Qt Key_A..Key_Z match ASCII; only allow A–Z (app shortcuts are letter-based).
    if (actualKey < Qt::Key_A || actualKey > Qt::Key_Z)
        return;
    const UINT vk = static_cast<UINT>('A' + (actualKey - Qt::Key_A));

    const int hotkeyId = next_hotkey_id_++;
    if (RegisterHotKey(nullptr, hotkeyId, modifiers, vk)) {
        registered_hotkeys_[hotkeyId] = HotkeyData{hotkeyId, actionId};
    }
#endif

#ifdef Q_OS_LINUX
    if (!x11_display_) return;
    Display *display = static_cast<Display *>(x11_display_);

    int key = keySeq[0].toCombined();
    unsigned int modifiers = 0;
    if (key & Qt::CTRL)  modifiers |= ControlMask;
    if (key & Qt::ALT)   modifiers |= Mod1Mask;
    if (key & Qt::SHIFT) modifiers |= ShiftMask;
    if (key & Qt::META)  modifiers |= Mod4Mask;

    int actualKey = key & 0xFFFF;

    // Qt's key codes for A-Z and 0-9 are defined to match their ASCII
    // values, which in turn match the corresponding X11 KeySym values, so
    // we can translate directly rather than maintaining a lookup table.
    KeySym keysym = NoSymbol;
    if (actualKey >= Qt::Key_A && actualKey <= Qt::Key_Z) {
        keysym = static_cast<KeySym>(actualKey);
    } else if (actualKey >= Qt::Key_0 && actualKey <= Qt::Key_9) {
        keysym = static_cast<KeySym>(actualKey);
    } else if (actualKey >= Qt::Key_F1 && actualKey <= Qt::Key_F35) {
        keysym = XK_F1 + (actualKey - Qt::Key_F1);
    } else {
        return; // Unsupported key for now
    }

    KeyCode keycode = XKeysymToKeycode(display, keysym);
    if (keycode == 0) return;

    for (unsigned int lockMask : kLockMaskCombos) {
        XGrabKey(display, keycode, modifiers | lockMask, x11_root_, True, GrabModeAsync, GrabModeAsync);
    }
    XFlush(display);

    X11HotkeyData data;
    data.keycode = keycode;
    data.modifiers = modifiers;
    data.actionId = actionId;
    x11_hotkeys_.append(data);
#endif
}

void GlobalHotkeyListener::unregisterAllHotkeys() {
#ifdef Q_OS_WIN
    for (const auto &data : registered_hotkeys_) {
        UnregisterHotKey(NULL, data.id);
    }
    registered_hotkeys_.clear();
#endif

#ifdef Q_OS_LINUX
    if (x11_display_) {
        Display *display = static_cast<Display *>(x11_display_);
        for (const auto &data : x11_hotkeys_) {
            for (unsigned int lockMask : kLockMaskCombos) {
                XUngrabKey(display, data.keycode, data.modifiers | lockMask, x11_root_);
            }
        }
        XFlush(display);
    }
    x11_hotkeys_.clear();
#endif
}

#ifdef Q_OS_LINUX
void GlobalHotkeyListener::on_x11_activity() {
    if (!x11_display_) return;
    Display *display = static_cast<Display *>(x11_display_);

    while (XPending(display)) {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == KeyPress) {
            // Ignore the lock-modifier bits when comparing, since we grabbed
            // every combination of them for the same logical hotkey.
            unsigned int mods = event.xkey.state & (ControlMask | Mod1Mask | ShiftMask | Mod4Mask);
            for (const auto &data : x11_hotkeys_) {
                if (data.keycode == event.xkey.keycode && data.modifiers == mods) {
                    emit globalHotkeyPressed(data.actionId);
                    break;
                }
            }
        }
    }
}

// Xlib.h defines terse macros (None, Bool, True, False, Status, KeyPress...)
// that would otherwise leak into and collide with the rest of this
// translation unit. Undefine them now that the X11-specific code above is
// done using them.
#undef None
#undef Bool
#undef True
#undef False
#undef Status
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#endif
