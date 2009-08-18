/**
 * @file remotecall.h
 * @brief RemoteCall class
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 10 Jul 2009
 */
#ifndef _RemoteCall_H
#define _RemoteCall_H

#include <QtCore>

namespace qrs {

   /**
    * @brief Remote call representation
    *
    * This class contains all necessary information about slot to be called.
    * It's used by the QRemoteSignal library as internall remote call
    * representation. It's serealized to a string or deserealized from a
    * string by one of AbsRemoteCallSerializer derived classes.
    *
    * @sa AbsRemoteCallSerializer
    */
   class RemoteCall {
      public:
         RemoteCall() {};
         ~RemoteCall() {};

         /**
          * @brief Sets service name
          * @param val new service name
          * @sa service
          */
         void setService ( const QString& val ) {service = val;};
         /**
          * @brief Returns service name
          * @return current service name
          * @sa service
          */
         const QString& getService() const {return service;};

         /**
          * @brief Sets method name
          * @param val new method name
          * @sa method
          */
         void setMethod ( const QString& val ) {method = val;};
         /**
          * @brief Returns method name
          * @return current method name
          * @sa method
          */
         const QString& getMethod() const {return method;}

         const QMap<QString,QVariant>& params() const {return paramsMap;};
         QMap<QString,QVariant>& params() {return paramsMap;};

      private:
         /**
          * @brief service name
          *
          * This property contains a service name which is going to be called.
          *
          * Service is one of the base concepts of the QRemoteSignal library.
          * It's used to group several methods in a single unit. Each method
          * your application share should be a part of some service. There
          * is no limitations on the number of services in a single
          * application.
          *
          * @sa getService
          * @sa setService
          */
         QString service;
         /**
          * @brief method name
          *
          * This property contains a name of the method to be called
          *
          * @sa getMethod
          * @sa setMethod
          */
         QString method;
         QMap<QString,QVariant> paramsMap;
   };

}

#endif
