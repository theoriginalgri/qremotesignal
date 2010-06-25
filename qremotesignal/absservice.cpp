/**
 * @file absservice.cpp
 * @brief AbsService class methods definitions
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 09 May 2010
 */
#include "absservice.h"
#include <QtCore/QtGlobal>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaMethod>
#include <QtCore/QByteArray>

using namespace qrs;

bool AbsService::autoconnect(QObject *target) {
    const QMetaObject *serviceMetaObject = this->metaObject();
    const QMetaObject *targetMetaObject = target->metaObject();
    bool res = true;
    for ( int i = serviceMetaObject->methodOffset();
          i < serviceMetaObject->methodCount();
          i++ ) {
        QMetaMethod method = serviceMetaObject->method(i);
        QByteArray nsignature = QMetaObject::normalizedSignature(method.signature());
        if ( method.methodType() == QMetaMethod::Signal ) {
            int pairIndx = targetMetaObject->indexOfSlot(nsignature);
            if (pairIndx == -1) {
                res = false;
                continue;
            }
            QMetaMethod pair = targetMetaObject->method(pairIndx);
#if (QT_VERSION >= QT_VERSION_CHECK(4,8,0))
            connect(this,method,target,pair);
#else
            // Some kluges which relies on Qt4 SIGNAL and SLOT macro implementation
            QByteArray signal;
            signal.reserve(qstrlen(method.signature())+1);
            signal.append((char)(QSIGNAL_CODE + '0'));
            signal.append(method.signature());

            QByteArray slot;
            slot.reserve(qstrlen(pair.signature())+1);
            slot.append((char)(QSLOT_CODE + '0'));
            slot.append(pair.signature());

            connect(this,signal.constData(), target,slot.constData());
#endif
        } else if ( method.methodType() == QMetaMethod::Slot ) {
            int pairIndx = targetMetaObject->indexOfSignal(nsignature);
            if (pairIndx == -1) {
                res = false;
                continue;
            }
            QMetaMethod pair = targetMetaObject->method(pairIndx);
#if (QT_VERSION >= QT_VERSION_CHECK(4,8,0))
            connect(target,pair,this,method);
#else
            // Some kluges which relies on Qt4 SIGNAL and SLOT macro implementation
            QByteArray signal;
            signal.reserve(qstrlen(pair.signature())+1);
            signal.append((char)(QSIGNAL_CODE + '0'));
            signal.append(pair.signature());

            QByteArray slot;
            slot.reserve(qstrlen(method.signature())+1);
            slot.append((char)(QSLOT_CODE + '0'));
            slot.append(method.signature());

            connect(target,signal.constData(), this,slot.constData());
#endif
        } else {
            // Nothing to do with other types of methods
            continue;
        }
    }
    return res;
}