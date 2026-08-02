#ifndef QT_FIXES_H
#define QT_FIXES_H

// Guard against accidental macro/typedef collisions that redefine QEvent
#ifdef QEvent
#undef QEvent
#endif

#include <QEvent>

#endif // QT_FIXES_H
