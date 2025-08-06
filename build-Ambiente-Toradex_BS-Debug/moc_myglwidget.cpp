/****************************************************************************
** Meta object code from reading C++ file 'myglwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ambiente/myglwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'myglwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MyGLWidget_t {
    QByteArrayData data[26];
    char stringdata0[265];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyGLWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyGLWidget_t qt_meta_stringdata_MyGLWidget = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MyGLWidget"
QT_MOC_LITERAL(1, 11, 10), // "fpsUpdated"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 3), // "fps"
QT_MOC_LITERAL(4, 27, 11), // "tempUpdated"
QT_MOC_LITERAL(5, 39, 4), // "temp"
QT_MOC_LITERAL(6, 44, 9), // "kmUpdated"
QT_MOC_LITERAL(7, 54, 2), // "km"
QT_MOC_LITERAL(8, 57, 17), // "coordinatesUpdate"
QT_MOC_LITERAL(9, 75, 3), // "lon"
QT_MOC_LITERAL(10, 79, 3), // "lat"
QT_MOC_LITERAL(11, 83, 21), // "movementStatusUpdated"
QT_MOC_LITERAL(12, 105, 6), // "status"
QT_MOC_LITERAL(13, 112, 16), // "immStatusUpdated"
QT_MOC_LITERAL(14, 129, 8), // "probReta"
QT_MOC_LITERAL(15, 138, 9), // "probCurva"
QT_MOC_LITERAL(16, 148, 16), // "onRtkModeChanged"
QT_MOC_LITERAL(17, 165, 7), // "newMode"
QT_MOC_LITERAL(18, 173, 8), // "gameTick"
QT_MOC_LITERAL(19, 182, 13), // "onSpeedUpdate"
QT_MOC_LITERAL(20, 196, 8), // "newSpeed"
QT_MOC_LITERAL(21, 205, 16), // "onSteeringUpdate"
QT_MOC_LITERAL(22, 222, 13), // "steeringValue"
QT_MOC_LITERAL(23, 236, 15), // "onGpsDataUpdate"
QT_MOC_LITERAL(24, 252, 7), // "GpsData"
QT_MOC_LITERAL(25, 260, 4) // "data"

    },
    "MyGLWidget\0fpsUpdated\0\0fps\0tempUpdated\0"
    "temp\0kmUpdated\0km\0coordinatesUpdate\0"
    "lon\0lat\0movementStatusUpdated\0status\0"
    "immStatusUpdated\0probReta\0probCurva\0"
    "onRtkModeChanged\0newMode\0gameTick\0"
    "onSpeedUpdate\0newSpeed\0onSteeringUpdate\0"
    "steeringValue\0onGpsDataUpdate\0GpsData\0"
    "data"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyGLWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       4,    1,   72,    2, 0x06 /* Public */,
       6,    1,   75,    2, 0x06 /* Public */,
       8,    2,   78,    2, 0x06 /* Public */,
      11,    1,   83,    2, 0x06 /* Public */,
      13,    3,   86,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    1,   93,    2, 0x0a /* Public */,
      18,    0,   96,    2, 0x08 /* Private */,
      19,    1,   97,    2, 0x08 /* Private */,
      21,    1,  100,    2, 0x08 /* Private */,
      23,    1,  103,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Float,    5,
    QMetaType::Void, QMetaType::Float,    7,
    QMetaType::Void, QMetaType::Float, QMetaType::Float,    9,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString, QMetaType::Double, QMetaType::Double,   12,   14,   15,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,   20,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, 0x80000000 | 24,   25,

       0        // eod
};

void MyGLWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MyGLWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->fpsUpdated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->tempUpdated((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->kmUpdated((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: _t->coordinatesUpdate((*reinterpret_cast< float(*)>(_a[1])),(*reinterpret_cast< float(*)>(_a[2]))); break;
        case 4: _t->movementStatusUpdated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->immStatusUpdated((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 6: _t->onRtkModeChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->gameTick(); break;
        case 8: _t->onSpeedUpdate((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 9: _t->onSteeringUpdate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->onGpsDataUpdate((*reinterpret_cast< const GpsData(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MyGLWidget::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGLWidget::fpsUpdated)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MyGLWidget::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGLWidget::tempUpdated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MyGLWidget::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGLWidget::kmUpdated)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MyGLWidget::*)(float , float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGLWidget::coordinatesUpdate)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MyGLWidget::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGLWidget::movementStatusUpdated)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (MyGLWidget::*)(const QString & , double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGLWidget::immStatusUpdated)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MyGLWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_meta_stringdata_MyGLWidget.data,
    qt_meta_data_MyGLWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MyGLWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyGLWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MyGLWidget.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QOpenGLFunctions"))
        return static_cast< QOpenGLFunctions*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int MyGLWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void MyGLWidget::fpsUpdated(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MyGLWidget::tempUpdated(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void MyGLWidget::kmUpdated(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MyGLWidget::coordinatesUpdate(float _t1, float _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void MyGLWidget::movementStatusUpdated(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void MyGLWidget::immStatusUpdated(const QString & _t1, double _t2, double _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
