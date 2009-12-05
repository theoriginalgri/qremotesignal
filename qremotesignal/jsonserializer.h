/**
 * @file jsonserializer.h
 * @brief JsonSerializer interface
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 5 Sep 2009
 */
#ifndef qrsjsonserializer_h
#define qrsjsonserializer_h

#include "qrsexport.h"
#include "absmessageserializer.h"

namespace qrs {

   /**
    * @brief Message serializer for simple JSON based remote call protocol
    *
    * This class serializes internal library message representation to a simple
    * JSON based protocol message.
    *
    * Top level element of JSON object can be @b RemoteCall or @b Error it
    * contains child JSON object with detailed information about slot to be
    * called or about error.
    *
    * @section RemoteCall Remote call JSON representation.
    *
    * Remote call message contains the following elements in the child JSON
    * object:
    * @li @b service element containing string with destination service name.
    * @li @b method element containing string with destination slot name.
    * @li @b params element containing map of slot parameters. Each parameterhas
    * a name which is stored as a key in this map.
    *
    * Here is an example of a remote call message:
    * @code
    * {
    *    "RemoteCall" : {
    *       "service" : "Example" ,
    *       "method" : "foo" ,
    *       "params" : {
    *          "num" : 128
    *       }
    *    }
    * }
    * @endcode
    * This message is request to call slot @b foo in the @b Example service
    * with single parameters' @b num value 128.
    *
    * There is no information about parameter type and if it is
    * possible to convert parameters' value to the required type then no error
    * will occure during processing this message. For example if service
    * @b Example has a slot @b foo with the following definition:
    * @code
    * void foo(const QString &num);
    * @endcode
    * then value in the JSON message will be converted to string "128".
    *
    * Conversations of values are done by convertor functions:
    * @code
    * QVariant qrs::createArg(const YourType &val);
    * bool qrs::getArgValue(const QVariant &arg, YourType& res);
    * @endcode
    * You can find more information about them in the @ref converters article.
    *
    * @section Error Error message JSON representation.
    *
    * Error message contains the following elements in the child JSON object:
    * @li @b errorCode element containing integer code of error. Possible
    * values are listed in the Message::ErrorType enum.
    * @li @b description element containing human readable error description.
    * @li Optional @b service element containing the name of a service where
    * error occured. For example error message with this element may be
    * returned in responce on remote call of slot in some service if
    * destination service doesn't contain requested slot.
    * @li Optional @b method element with the name of a method where error
    * occured. Error message can contain this element only if it also contains
    * @b service element.
    *
    * Here is example of error message JSON representation:
    * @code
    * {
    *    "Error" : {
    *       "errorCode" : 4,
    *       "description" : "Unknown method wrong",
    *       "service" : "Example",
    *       "method" : "wrong"
    *    }
    * }
    * @endcode
    *
    * @note JSON object representing error generated by this serializer always
    * contains @b service and @b method elements even if they are not specified
    * they contains empty strings. However this serializer can understand error
    * messages without optional elements so there is no need to specify them if
    * you want to call slot in application which is using this serializer for
    * remote calls from application which is not using QRemoteSignal library.
    */
   class QRS_EXPORT JsonSerializer: public AbsMessageSerializer {
      Q_OBJECT
      public:
         explicit JsonSerializer (QObject *parent=0):AbsMessageSerializer(parent) {};
         virtual ~JsonSerializer() {};

         virtual QByteArray serialize ( const Message& msg )
               throw(UnsupportedTypeException);
         virtual MessageAP deserialize ( const QByteArray& msg )
               throw(MessageParsingException);
      private:
         Q_DISABLE_COPY(JsonSerializer);
   };

}

#endif