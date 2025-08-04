/****************************************************************************
** Meta object code from reading C++ file 'gpsfileplayer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../Ambiente/gpsfileplayer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'gpsfileplayer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GpsFilePlayer_t {
    QByteArrayData data[11];
    char stringdata0[121];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GpsFilePlayer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GpsFilePlayer_t qt_meta_stringdata_GpsFilePlayer = {
    {
QT_MOC_LITERAL(0, 0, 13), // "GpsFilePlayer"
QT_MOC_LITERAL(1, 14, 13), // "gpsDataUpdate"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 7), // "GpsData"
QT_MOC_LITERAL(4, 37, 4), // "data"
QT_MOC_LITERAL(5, 42, 16), // "playbackFinished"
QT_MOC_LITERAL(6, 59, 13), // "startPlayback"
QT_MOC_LITERAL(7, 73, 8), // "filePath"
QT_MOC_LITERAL(8, 82, 9), // "intervaMs"
QT_MOC_LITERAL(9, 92, 12), // "stopPlayback"
QT_MOC_LITERAL(10, 105, 15) // "processNextLine"

    },
    "GpsFilePlayer\0gpsDataUpdate\0\0GpsData\0"
    "data\0playbackFinished\0startPlayback\0"
    "filePath\0intervaMs\0stopPlayback\0"
    "processNextLine"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GpsFilePlayer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       5,    0,   47,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    2,   48,    2, 0x0a /* Public */,
       6,    1,   53,    2, 0x2a /* Public | MethodCloned */,
       9,    0,   56,    2, 0x0a /* Public */,
      10,    0,   57,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    7,    8,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void GpsFilePlayer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GpsFilePlayer *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->gpsDataUpdate((*reinterpret_cast< const GpsData(*)>(_a[1]))); break;
        case 1: _t->playbackFinished(); break;
        case 2: _t->startPlayback((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->startPlayback((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->stopPlayback(); break;
        case 5: _t->processNextLine(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GpsFilePlayer::*)(const GpsData & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GpsFilePlayer::gpsDataUpdate)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GpsFilePlayer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GpsFilePlayer::playbackFinished)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GpsFilePlayer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_GpsFilePlayer.data,
    qt_meta_data_GpsFilePlayer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GpsFilePlayer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GpsFilePlayer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GpsFilePlayer.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GpsFilePlayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void GpsFilePlayer::gpsDataUpdate(const GpsData & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GpsFilePlayer::playbackFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
