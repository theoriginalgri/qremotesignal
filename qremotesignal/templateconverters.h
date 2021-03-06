/**
 * @file templateconverters.h
 * @brief Template QList and QString converters
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 22 Jul 2010
 */
#ifndef _TemplateConverters_H
#define _TemplateConverters_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QVariant>

namespace qrs {

    // ----- QList -----
    template<typename T>
    QVariant createArg(const QList<T>& val) {
        QVariantList res;
        foreach(T t, val) {
            res.append( createArg(t) );
        }
        return QVariant(res);
    }

    template<typename T>
    bool getArgValue(const QVariant& arg, QList<T>& res) {
        if( !arg.canConvert<QVariantList>() ){
            return false;
        }
        QVariantList argContent = arg.toList();
        res.clear();
        foreach(QVariant item, argContent) {
            T t;
            if ( ! getArgValue(item, t) ) {
                return false;
            }
            res.append(t);
        }
        return true;
    }

    // ----- QMap -----
    template<typename T>
    QVariant createArg(const QMap<QString,T>& val) {
        QVariantMap res;
        typename QMap<QString,T>::const_iterator indx = val.begin();
        while ( indx != val.end() ) {
            res[indx.key()] = createArg(indx.value());
            indx++;
        }
        return QVariant(res);
    }

    template<typename T>
    bool getArgValue(const QVariant& arg, QMap<QString,T>& res) {
        if( !arg.canConvert<QVariantMap>() ){
            return false;
        }
        QVariantMap argContent = arg.toMap();
        res.clear();
        QVariantMap::const_iterator indx = argContent.begin();
        while ( indx != argContent.end() ) {
            T t;
            if ( !getArgValue(indx.value() , t) ) {
                return false;
            }
            res[indx.key()] = t;
            indx++;
        }
        return true;
    }

}

#endif
