/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ambiente/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[15];
    char stringdata0[156];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 14), // "updateFpsLabel"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 3), // "fps"
QT_MOC_LITERAL(4, 31, 15), // "updateTempLabel"
QT_MOC_LITERAL(5, 47, 4), // "temp"
QT_MOC_LITERAL(6, 52, 13), // "updateKmLabel"
QT_MOC_LITERAL(7, 66, 2), // "km"
QT_MOC_LITERAL(8, 69, 22), // "updateCoordinatesLabel"
QT_MOC_LITERAL(9, 92, 3), // "lon"
QT_MOC_LITERAL(10, 96, 3), // "lat"
QT_MOC_LITERAL(11, 100, 25), // "updateMovementStatusLabel"
QT_MOC_LITERAL(12, 126, 6), // "status"
QT_MOC_LITERAL(13, 133, 16), // "onProfileChanged"
QT_MOC_LITERAL(14, 150, 5) // "index"

    },
    "MainWindow\0updateFpsLabel\0\0fps\0"
    "updateTempLabel\0temp\0updateKmLabel\0"
    "km\0updateCoordinatesLabel\0lon\0lat\0"
    "updateMovementStatusLabel\0status\0"
    "onProfileChanged\0index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x08 /* Private */,
       4,    1,   47,    2, 0x08 /* Private */,
       6,    1,   50,    2, 0x08 /* Private */,
       8,    2,   53,    2, 0x08 /* Private */,
      11,    1,   58,    2, 0x08 /* Private */,
      13,    1,   61,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Float,    5,
    QMetaType::Void, QMetaType::Float,    7,
    QMetaType::Void, QMetaType::Float, QMetaType::Float,    9,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::Int,   14,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->updateFpsLabel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->updateTempLabel((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->updateKmLabel((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: _t->updateCoordinatesLabel((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 4: _t->updateMovementStatusLabel((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->onProfileChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
