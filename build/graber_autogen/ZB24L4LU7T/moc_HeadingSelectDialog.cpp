/****************************************************************************
** Meta object code from reading C++ file 'HeadingSelectDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ui/dialogs/HeadingSelectDialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HeadingSelectDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_HeadingSelectDialog_t {
    uint offsetsAndSizes[18];
    char stringdata0[20];
    char stringdata1[23];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[23];
    char stringdata5[17];
    char stringdata6[5];
    char stringdata7[18];
    char stringdata8[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_HeadingSelectDialog_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_HeadingSelectDialog_t qt_meta_stringdata_HeadingSelectDialog = {
    {
        QT_MOC_LITERAL(0, 19),  // "HeadingSelectDialog"
        QT_MOC_LITERAL(20, 22),  // "on_search_text_changed"
        QT_MOC_LITERAL(43, 0),  // ""
        QT_MOC_LITERAL(44, 4),  // "text"
        QT_MOC_LITERAL(49, 22),  // "on_item_double_clicked"
        QT_MOC_LITERAL(72, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(89, 4),  // "item"
        QT_MOC_LITERAL(94, 17),  // "on_select_clicked"
        QT_MOC_LITERAL(112, 17)   // "on_debounce_timer"
    },
    "HeadingSelectDialog",
    "on_search_text_changed",
    "",
    "text",
    "on_item_double_clicked",
    "QListWidgetItem*",
    "item",
    "on_select_clicked",
    "on_debounce_timer"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_HeadingSelectDialog[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x08,    1 /* Private */,
       4,    1,   41,    2, 0x08,    3 /* Private */,
       7,    0,   44,    2, 0x08,    5 /* Private */,
       8,    0,   45,    2, 0x08,    6 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject HeadingSelectDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_HeadingSelectDialog.offsetsAndSizes,
    qt_meta_data_HeadingSelectDialog,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_HeadingSelectDialog_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<HeadingSelectDialog, std::true_type>,
        // method 'on_search_text_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'on_item_double_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'on_select_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_debounce_timer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void HeadingSelectDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HeadingSelectDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_search_text_changed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->on_item_double_clicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 2: _t->on_select_clicked(); break;
        case 3: _t->on_debounce_timer(); break;
        default: ;
        }
    }
}

const QMetaObject *HeadingSelectDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HeadingSelectDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_HeadingSelectDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int HeadingSelectDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
