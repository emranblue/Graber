/****************************************************************************
** Meta object code from reading C++ file 'InjectFromNoteDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../ui/dialogs/InjectFromNoteDialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'InjectFromNoteDialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_InjectFromNoteDialog_t {
    uint offsetsAndSizes[18];
    char stringdata0[21];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[15];
    char stringdata5[23];
    char stringdata6[17];
    char stringdata7[5];
    char stringdata8[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_InjectFromNoteDialog_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_InjectFromNoteDialog_t qt_meta_stringdata_InjectFromNoteDialog = {
    {
        QT_MOC_LITERAL(0, 20),  // "InjectFromNoteDialog"
        QT_MOC_LITERAL(21, 19),  // "onFileFilterChanged"
        QT_MOC_LITERAL(41, 0),  // ""
        QT_MOC_LITERAL(42, 4),  // "text"
        QT_MOC_LITERAL(47, 14),  // "onFileSelected"
        QT_MOC_LITERAL(62, 22),  // "onHeadingDoubleClicked"
        QT_MOC_LITERAL(85, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(102, 4),  // "item"
        QT_MOC_LITERAL(107, 15)   // "onInjectClicked"
    },
    "InjectFromNoteDialog",
    "onFileFilterChanged",
    "",
    "text",
    "onFileSelected",
    "onHeadingDoubleClicked",
    "QListWidgetItem*",
    "item",
    "onInjectClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_InjectFromNoteDialog[] = {

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
       4,    0,   41,    2, 0x08,    3 /* Private */,
       5,    1,   42,    2, 0x08,    4 /* Private */,
       8,    0,   45,    2, 0x08,    6 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject InjectFromNoteDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_InjectFromNoteDialog.offsetsAndSizes,
    qt_meta_data_InjectFromNoteDialog,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_InjectFromNoteDialog_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<InjectFromNoteDialog, std::true_type>,
        // method 'onFileFilterChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onFileSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onHeadingDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onInjectClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void InjectFromNoteDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<InjectFromNoteDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onFileFilterChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->onFileSelected(); break;
        case 2: _t->onHeadingDoubleClicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 3: _t->onInjectClicked(); break;
        default: ;
        }
    }
}

const QMetaObject *InjectFromNoteDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *InjectFromNoteDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_InjectFromNoteDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int InjectFromNoteDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
