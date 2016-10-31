/****************************************************************************
** Meta object code from reading C++ file 'QVX_TensileTest.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../VoxCad/QVX_TensileTest.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QVX_TensileTest.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QVX_TensileTest[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x05,
      44,   16,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
     127,   67,   16,   16, 0x0a,
     251,  201,   16,   16, 0x2a,
     360,  318,   16,   16, 0x2a,
     443,  413,   16,   16, 0x2a,
     498,  489,   16,   16, 0x0a,
     523,   16,   16,   16, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_QVX_TensileTest[] = {
    "QVX_TensileTest\0\0StartExternalGLUpdate(int)\0"
    "StopExternalGLUpdate()\0"
    "pEnvIn,NumStepIn,ConvThreshIn,MixRadiusIn,ModelIn,PolyExpIn\0"
    "BeginTensileTest(QVX_Environment*,int,double,double,MatBlendModel,doub"
    "le)\0"
    "pEnvIn,NumStepIn,ConvThreshIn,MixRadiusIn,ModelIn\0"
    "BeginTensileTest(QVX_Environment*,int,double,double,MatBlendModel)\0"
    "pEnvIn,NumStepIn,ConvThreshIn,MixRadiusIn\0"
    "BeginTensileTest(QVX_Environment*,int,double,double)\0"
    "pEnvIn,NumStepIn,ConvThreshIn\0"
    "BeginTensileTest(QVX_Environment*,int,double)\0"
    "pMessage\0RunTensileTest(QString*)\0"
    "RunTensileTest()\0"
};

void QVX_TensileTest::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QVX_TensileTest *_t = static_cast<QVX_TensileTest *>(_o);
        switch (_id) {
        case 0: _t->StartExternalGLUpdate((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->StopExternalGLUpdate(); break;
        case 2: _t->BeginTensileTest((*reinterpret_cast< QVX_Environment*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< MatBlendModel(*)>(_a[5])),(*reinterpret_cast< double(*)>(_a[6]))); break;
        case 3: _t->BeginTensileTest((*reinterpret_cast< QVX_Environment*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< MatBlendModel(*)>(_a[5]))); break;
        case 4: _t->BeginTensileTest((*reinterpret_cast< QVX_Environment*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        case 5: _t->BeginTensileTest((*reinterpret_cast< QVX_Environment*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 6: _t->RunTensileTest((*reinterpret_cast< QString*(*)>(_a[1]))); break;
        case 7: _t->RunTensileTest(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QVX_TensileTest::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QVX_TensileTest::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QVX_TensileTest,
      qt_meta_data_QVX_TensileTest, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QVX_TensileTest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QVX_TensileTest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QVX_TensileTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QVX_TensileTest))
        return static_cast<void*>(const_cast< QVX_TensileTest*>(this));
    if (!strcmp(_clname, "CVX_Sim"))
        return static_cast< CVX_Sim*>(const_cast< QVX_TensileTest*>(this));
    return QWidget::qt_metacast(_clname);
}

int QVX_TensileTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void QVX_TensileTest::StartExternalGLUpdate(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QVX_TensileTest::StopExternalGLUpdate()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
