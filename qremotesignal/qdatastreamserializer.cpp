/**
 * @file qdatastreamserializer.cpp
 * @brief QDataStreamSerializer implementation
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 27 Jan 2010
 */
#include "qdatastreamserializer.h"

#include <QtCore/QBuffer>

using namespace qrs;

QDataStream &operator<<(QDataStream &stream, const Message &msg) {
    qint8 type,errorType;
    type = (qint8)msg.type();
    errorType = (qint8)msg.errorType();
    stream << type << errorType;
    stream << msg.error();
    stream << msg.service() << msg.method();
    stream << msg.params();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, Message &msg) {
    qint8 type,errorType;
    QString error,service,method;
    QVariantMap params;
    stream >> type >> errorType;
    stream >> error;
    stream >> service >> method;
    stream >> params;
    msg.setType((Message::MsgType)type);
    msg.setErrorType((Message::ErrorType)errorType);
    msg.setError(error);
    msg.setService(service);
    msg.setMethod(method);
    msg.setParams(params);
    return stream;
}
