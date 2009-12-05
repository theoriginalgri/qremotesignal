/**
 * @file servicesmanager.cpp
 * @brief ServicesManager class
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 10 Jul 2009
 */
#include "servicesmanager.h"

using namespace qrs;

/**
 * Constructs new ServicesManager object. No serializer set. You need to set
 * one with setSerializer function.
 */
ServicesManager::ServicesManager(QObject *parent): QObject(parent) {
   mSerializer = 0;
}

/**
 * @brief Process received raw message
 *
 * This slot can be connected to some signal of class produces raw messages. It
 * process received message and calls corresponding service message processor.
 *
 * If message require to call a method from a service which is not registered
 * in this ServicesManager it sends error message by emiting send(QByteArray)
 * signal. Error message will be send if a service which method is called can't
 * process the message received (no such method, wrong arguments or arguments
 * types).
 *
 * This slot do nothing if serializer to serialize/deserialize raw messages is
 * not set.
 *
 * @sa send(QByteArray)
 * @sa AbsMessageSerializer
 *
 * @param msg received raw message
 */
void ServicesManager::receive(const QByteArray& msg) {
   if ( !mSerializer ) {
      return;
   }
   MessageAP message;
   try {
      message = mSerializer->deserialize(msg);
   } catch (const MessageParsingException& e) {
      Message err;
      err.setErrorType(e.mErrorType);
      err.setError( e.reason() );
      emit mSerializer->serialize(err);
      return;
   }
   if ( message->type() == Message::Error ) {
      emit error(this, message->errorType(), message->error());
      return;
   }
   QMap<QString,AbsService*>::iterator res = mServices.find(message->service());
   if ( res != mServices.end() ) {
      try {
         (*res)->processMessage(*message);
      } catch ( IncorrectMethodException& e ) {
         Message err;
         err.setErrorType(Message::IncorrectMethod);
         err.setError(e.reason());
         err.setService(message->service());
         err.setMethod(message->method());
         emit send( mSerializer->serialize(err) );
         return;
      }
   } else {
      Message err;
      err.setErrorType(Message::UnknownService);
      err.setError(QString("Unknown service: \"%1\"").arg(message->service()));
      err.setService(message->service());
      emit send( mSerializer->serialize(err) );
      return;
   }
}

/**
 * This function registers new service or client in this ServicesManager. If
 * service with the same name have been already registerd it replace old
 * service object with the new one.
 *
 * @note One instance of service can be registered only in one services
 * manager. If instance you are passing to this method is registered in another
 * services manager it will be automatically unregistered there.
 *
 * Service name is determined with AbsService::name() function.
 *
 * @sa AbsService
 *
 * @param service service or client instance to be registered.
 */
void ServicesManager::registerService(AbsService* service) {
   if ( service->manager() != 0 ) {
      service->manager()->unregister(service);
   }
   mServices[service->name()] = service;
   service->setManager(this);
}

/**
 * @param name service or client name to unregister.
 *
 * Tthis function unregisters instance with the name given in parameter from
 * this services manager. If there is no client or service with theis name
 * registered in this manager it will do nothing.
 *
 * @return instance being unregistered or 0 if no instance with this name is
 * registered in this manager.
 *
 * If you are using Qt parent/child memory management mechanism you may want
 * to delete returned instance since in other case it will exists untill the
 * this instance of ServicesManager class exits.
 */
AbsService *ServicesManager::unregister(const QString &name) {
   QMap<QString, AbsService*>::iterator it = mServices.find(name);
   AbsService *res = 0;
   if ( it != mServices.end() ) {
      res = it.value();
      mServices.erase(it);
      res->setManager(0);
   }
   return res;
}

/**
 * @param instance service or client instance to be unregistered
 *
 * This function unregisters instance from this manager if it is registered in
 * other case it is doing nothing.
 */
void ServicesManager::unregister(AbsService *instance) {
   QMap<QString, AbsService*>::iterator it = mServices.find(instance->name());
   if ( it != mServices.end() && it.value() == instance ) {
      mServices.erase(it);
      instance->setManager(0);
   }
}

/**
 * @internal
 *
 * This function provided to be used by client classes generated from service
 * interface description. You should not use this function manually. It can be
 * renamed or removed in future versions.
 */
void ServicesManager::send(const Message& msg) {
   if ( !mSerializer ) return;
   emit send( mSerializer->serialize(msg) );
}

/**
 * This member function provided for convenience.
 *
 * Adds device to be used to send/receive raw messages. You may add several
 * devices to one ServicesManager instance. In this case any outgoing message
 * will be sent to all devices. If device is deleted it will be automatically
 * removed from the list of devices added by this function.
 *
 * @note You should add device to the ServicesManager instance only after you
 * have registered all services you are planning to use with this instance and
 * set serializer. Otherwise if device already have some arrived messages they
 * may be not processed (if serializer is not set) or processed incorrectly
 * (if destination service have not yot been registered).
 *
 * @param dev device to be used for sending/receiving messages
 */
void ServicesManager::addDevice(QIODevice* dev) {
   foreach (const QSharedPointer<DeviceManager> dm, mDevManagers) {
      if ( dm->device() == dev ) {
         return;
      }
   }
   QSharedPointer<DeviceManager> dm( new DeviceManager() );
   connect(dm.data(),SIGNAL(received(QByteArray)),
           this,SLOT(receive(const QByteArray&)));
   connect(this,SIGNAL(send(QByteArray)),
           dm.data(),SLOT(send(const QByteArray&)));
   dm->setDevice(dev);
   mDevManagers.append(dm);
   connect(dev,SIGNAL(destroyed( QObject* )),
           this,SLOT(onDeviceDeleted(QObject*)));
}

/**
 * This slot handles resources cleanup if one of the devices used by this
 * services manager to read write messages is deleted.
 */
void ServicesManager::onDeviceDeleted(QObject* dev) {
   QList< QSharedPointer<DeviceManager> >::iterator it;
   for ( it = mDevManagers.begin(); it != mDevManagers.end(); it++ ) {
      QObject *currentDev = (*it)->device();
      // DeviceManager stores QPointer instead of normal pointers and it can
      // know that dev is deleted before this slot is called. That's why I
      // should check that currentDev is non-zero.
      if ( !currentDev || currentDev == dev ) {
         mDevManagers.erase(it);
         return;
      }
   }
}

/**
 * @mainpage
 *
 * @li @ref tutorial
 * @li @ref services_concept
 * @li @ref qrsc
 * @li @ref converters
 */

/**
 * @page tutorial Tutorial
 *
 * This article provides step by step tutorial of creation client-server
 * applications with the QRemoteSignal library. You can find compleate source
 * code of the example application described here in the @e examples/hello
 * directory of the libbrary source package.
 *
 * @section hello_service Application remote interface
 *
 * Lets create simple client-server application. The client can connect to the
 * server and send a name. The server will send the string "Hello "+name in
 * response on the message from client.
 *
 * First of all we need to create some services describing server remote
 * interface. For our small server it will be enough to have only one service.
 * Lets create XML file with the name @b hello.xml with the following content:
 * @code
 * <service name="Hello">
 *    <slot name="setName">
 *       <param name="name" type="QString"/>
 *    </slot>
 *    <signal name="hello">
 *       <param name="greetings" type="QString"/>
 *    </signal>
 * </service>
 * @endcode
 * It contains one slot @b setName which allows our client to send a name to
 * the server and one signal @b hello which allows our server to send greetings
 * string to our client.
 *
 * Now we can generate C++ classes which we can use in our application (see
 * @ref code_generation for details). We can do it with the following commands:
 * @code
 * qrsc --service --header helloservice.h --source helloservice.cpp hello.xml
 * qrsc --client --header helloclient.h --source helloclient.cpp hello.xml
 * @endcode
 * Or we can write some build script which will invoke this commands from build
 * system (some help on this topic you can find in @ref build_systems document).
 *
 * We will get 4 files:
 * @li @b helloservice.h and @b helloservice.cpp which we will use in the
 * server application.
 * @li @b helloclient.h and @b helloclient.cpp which we will use in the
 * client application.
 *
 * @section hello_server Server application
 *
 * Our server application will contain Server class which creates QTcpServer
 * listening some port and processing new incomming connections. After new
 * connection established it creates Connection class instance which will work
 * with the socket. This instance will be deleted if socket is disconnected.
 *
 * Main function and Server class contain no QRemoteSignal specific code it's
 * just usual QtNetwork library ussage. Here is the code:
 * @li main.cpp
 * @code
 * #include <QtCore/QCoreApplication>
 *
 * #include "server.h"
 *
 * int main(int argc, char** argv) {
 *    QCoreApplication app(argc,argv);
 *    Server server(8081);
 *    return app.exec();
 * }
 * @endcode
 * @li server.h
 * @code
 * #ifndef Server_H
 * #define Server_H
 *
 * #include <QtCore/QObject>
 * #include <QtNetwork/QTcpServer>
 *
 * class Server : public QObject {
 *    Q_OBJECT
 *    public:
 *       Server (quint32 port, QObject *parent = 0);
 *       virtual ~Server() {};
 *    public slots:
 *       void onNewConnection();
 *    private:
 *       QTcpServer *mTcpServer;
 * };
 *
 * #endif
 * @endcode
 * @li server.cpp
 * @code
 * #include "server.h"
 *
 * #include "connection.h"
 *
 * Server::Server(quint32 port, QObject *parent):QObject(parent) {
 *    mTcpServer = new QTcpServer(this);
 *    connect(mTcpServer,SIGNAL(newConnection()),
 *             this,SLOT(onNewConnection()));
 *    mTcpServer->listen(QHostAddress::Any,port);
 * }
 *
 * void Server::onNewConnection() {
 *    QTcpSocket *socket = mTcpServer->nextPendingConnection();
 *    if ( socket == 0 ) {
 *       return;
 *    }
 *    Connection *connection = new Connection(socket,this);
 * }
 * @endcode
 *
 * All magick is done in the @b Connection class. First of all in its
 * constructor we should create qrs::ServicesManager class instance. Then we
 * can create @b HelloService class instance and register it in our services
 * manager:
 * @code
 * qrs::ServicesManager *manager = new qrs::ServicesManager(this);
 * mService = new qrs::HelloService(manager);
 * @endcode
 * Now @b mService instance also become a child of @b manager instance and will
 * be deleted if @b manager is deleted. QRemoteSignal library actively uses Qt4
 * memory management features to simplify preventing memory leaks in you
 * application. Constructor generated for the @b HelloService class which takes
 * pointer to a qrs::ServicesManager class instance automatically registers
 * newly created @b HelloService instace in the qrs::ServicesManager instance
 * passed to constructor and set this manager instance to be its parent.
 *
 * Now we need to connect @b HelloService signals and slots to some signal and
 * slots in our application to provide meaningfull reaction on incomming
 * requests. For our small server it's enough to provide one slot which will
 * be connected to @b HelloService::setName signal. This slot will call
 * @b HelloService::hello slot with greetings string:
 * @code
 * void Connection::onSetName(const QString &name) {
 *    mService->hello( QString("Hello %1").arg(name) );
 * }
 * @endcode
 *
 * And in the @b Connection class constructor we need to connect @b setName
 * signal to this slot right after @b HelloService instance creation:
 * @code
 * connect(mService,SIGNAL(setName(QString)),
 *         this,SLOT(onSetName(const QString&)));
 * @endcode
 *
 * After that we need to set serializer to be used by @b manager and add socket
 * to list of devices used for sending/receiving raw messages:
 * @code
 * manager->setSerializer(serializer);
 * manager->addDevice(socket);
 * @endcode
 * @b serializer is a static pointer to a global qrs::JsonSerializer instance we
 * are using for all connections.
 *
 * Lets take a look on compleate @b Connection class code:
 * @li connection.h
 * @code
 * #ifndef Connection_H
 * #define Connection_H
 *
 * #include <QtCore/QObject>
 * #include <QtNetwork/QTcpSocket>
 *
 * #include <QRemoteSignal>
 *
 * #include "helloservice.h"
 *
 * class Connection : public QObject {
 *    Q_OBJECT
 *    public:
 *       Connection(QTcpSocket *socket, QObject *parent = 0);
 *       virtual ~Connection() {};
 *    private slots:
 *       void onSetName(const QString& name);
 *    private:
 *       qrs::HelloService *mService;
 *       static qrs::AbsMessageSerializer *serializer;
 * };
 *
 * #endif
 * @endcode
 * @li connection.cpp
 * @code
 * #include "connection.h"
 *
 * #include <QRemoteSignal>
 *
 * qrs::AbsMessageSerializer *Connection::serializer = 0;
 *
 * Connection::Connection (QTcpSocket *socket, QObject *parent):QObject(parent) {
 *    socket->setParent(this);
 *    connect(socket,SIGNAL(disconnected()),
 *            this,SLOT(deleteLater()));
 *
 *    qrs::ServicesManager *manager = new qrs::ServicesManager(this);
 *    mService = new qrs::HelloService(manager);
 *    connect(mService,SIGNAL(setName(QString)),
 *            this,SLOT(onSetName(const QString&)));
 *
 *    if ( serializer == 0 ) {
 *       serializer = new qrs::JsonSerializer(qApp);
 *    }
 *    manager->setSerializer(serializer);
 *    manager->addDevice(socket);
 * }
 *
 * void Connection::onSetName(const QString &name) {
 *    mService->hello( QString("Hello %1").arg(name) );
 * }
 * @endcode
 *
 * Server application is ready!
 *
 * @section hello_client Client application
 *
 * Our client application will be simple console application which takes two
 * command line parameters: host to connect to and your name. It sends your
 * name to the server, and waits for @b hello signal from the server. Once this
 * signal is received it prints received string and stops.
 *
 * Lets create @b Client class which takes connected socket in constructor and
 * creates qrs::ServicesManager using this socket to send/receive raw messages.
 * Also this class will have @b onHello slot which will listen for a @b hello
 * signal from the server. This class is similair to @b Connection class in the
 * server. Here is its code:
 * @li client.h
 * @code
 * #ifndef Client_H
 * #define Client_H
 *
 * #include <QtCore/QObject>
 * #include <QtCore/QString>
 * #include <QtCore/QtGlobal>
 *
 * #include "helloclient.h"
 *
 * class QTcpSocket;
 *
 * class Client: public QObject {
 *    Q_OBJECT
 *    public:
 *       explicit Client(QTcpSocket *socket, QObject *parent = 0);
 *       virtual ~Client() {};
 *       qrs::HelloClient *hello() {return mClient;}
 *    private slots:
 *       void onHello(const QString &greetings);
 *    private:
 *       qrs::HelloClient *mClient;
 *       QTcpSocket *mSocket;
 * };
 *
 * #endif
 * @endcode
 * @li client.cpp
 * @code
 * #include "client.h"
 *
 * #include <QtCore/QtDebug>
 * #include <QtNetwork/QTcpSocket>
 *
 * #include <QRemoteSignal>
 *
 * #include "helloclient.h"
 *
 * Client::Client(QTcpSocket *socket, QObject *parent ):QObject(parent) {
 *    socket->setParent(this);
 *    mSocket = socket;
 *
 *    qrs::ServicesManager *manager = new qrs::ServicesManager(this);
 *    mClient = new qrs::HelloClient(manager);
 *    connect(mClient,SIGNAL(hello(QString)),
 *            this,SLOT(onHello(const QString &)));
 *
 *    manager->setSerializer( new qrs::JsonSerializer(this) );
 *    manager->addDevice(mSocket);
 * }
 *
 * void Client::onHello(const QString &greetings) {
 *    qDebug() << greetings;
 *    mSocket->close();
 * }
 * @endcode
 *
 * We have created qrs::ServicesManager instance, created @b HelloClient
 * instance and registered it in manager, connected @b hello signal to slot
 * providing meaningfull reaction on incoming message everything like in the
 * @b Connection class of the server application.
 *
 * In our main function we will check command line arguments, connect to the
 * server, create @b Client class instance and send the name to the server:
 * @li main.cpp
 * @code
 * #include <QtCore/QCoreApplication>
 * #include <QtCore/QStringList>
 *
 * #include <QtCore/QtDebug>
 * #include <QtNetwork/QTcpSocket>
 *
 * #include "client.h"
 *
 * const int CONNECTION_TIMEOUT = 30000;
 *
 * int main(int argc, char** argv) {
 *    QCoreApplication app(argc,argv);
 *    QStringList args = app.arguments();
 *    if ( args.count() != 3 ) {
 *       qDebug() << "Ussage:";
 *       qDebug() << "\t" << args[0] << "host your_name";
 *       return 1;
 *    }
 *    QTcpSocket *socket = new QTcpSocket();
 *    QObject::connect(socket,SIGNAL(disconnected()),
 *                     qApp,SLOT(quit()));
 *    socket->connectToHost(args[1],8081);
 *    if ( ! socket->waitForConnected(CONNECTION_TIMEOUT) ) {
 *       qDebug() << "Could not connect to host" << args[1];
 *       qDebug() << socket->errorString();
 *       return 1;
 *    }
 *    Client client(socket);
 *    client.hello()->setName(args[2]);
 *    return app.exec();
 * }
 * @endcode
 *
 * Now our client applicaion ready! Lets run the server and and try it:
 * @code
 * vestnik@vestnik-laptop:~/Development/QRemoteSignal/examples$ ./hello/client/client localhost VestniK
 * "Hello VestniK"
 * vestnik@vestnik-laptop:~/Development/QRemoteSignal/examples$
 * @endcode
 *
 * @section extending_hello Extending example with more slots
 *
 * The most important thing I'm trying to implement in the QRemoteSignal
 * library is simple extesibility of client server applications with new remote
 * callable slots. Lets extend our application with one new function to see how
 * simple it to do.
 *
 * It's a bit annoying to search your server in the processes list and kill.
 * Lets add ability to shoutdown server application from the client (sure it's
 * not secure but this is simple "hello world" example).
 *
 * First of all we need to add one more slot to the @b hello.xml file:
 * @code
 * <service name="Hello">
 *    ...
 *    <slot name="quit"/>
 *    ...
 * </service>
 * @endcode
 * It takes no parameters and just shoutdown the server.
 *
 * Now we need to regenerate @b HelloService and @b HelloClient classes and
 * connect @b HelloService::quit signal to @b qApp->quit slot in the
 * @b Connection class constructor. Lets add the following lines after
 * @b HelloService instance creation:
 * @code
 * connect(mService,SIGNAL(quit()),
 *         qApp,SLOT(quit()));
 * @endcode
 *
 * Server now provides support of remote shoutdown feature!
 *
 * Lets modify our client to use this feature. We will just replece this line
 * in the main.cpp file of the client application:
 * @code
 * client.hello()->setName(args[2]);
 * @endcode
 * With this several lines:
 * @code
 * if ( args[2] == "-q" ) {
 *    client.hello()->quit();
 * } else {
 *    client.hello()->setName(args[2]);
 * }
 * @endcode
 * and that's all we have to do!
 */

/**
 * @page qrsc XML service description
 *
 * Slots you want to call remotely should be described in an XML file which
 * will be used to generate C++ classes you can use in your application. One
 * XML file describes one service (see @ref services_concept for details about
 * services).
 *
 * @section schema File format
 *
 * Here is a simple example of service XML description:
 * @code
 * <service name="Example">
 *    <customtypes header="customtypesconverters.h"/>
 *    <slot name="mySlot">
 *       <param name="num" type="qint32"/>
 *    </slot>
 *    <signal name="mySignal">
 *       <param name="str" type="QString"/>
 *    </signal>
 * </service>
 * @endcode
 *
 * @li Root element @b service must have @b name attribute with a service name.
 *
 * @li You can have any number of @b customtypes elements with @b header
 * attribute specifying C++ header file with convertors functions for your
 * custom types (see @ref converters for details). This header is included in
 * the *cpp files with the implementation of service and cient classes.
 *
 * @li @b slot element describes slot which can be called remotelly. Attribute
 * @b name specifies the name of this slot. This element can have any number of
 * child @b param elements.
 *
 * @li @b signal element describes signal which can be emited to remote
 * application. Attribute @b name specifies the name of this signal. This
 * element can have any number of child @b param elements.
 *
 * @li @b param element is used inside @b signal and @b slot elements to
 * describe signal or slot parameter. It has two attributes @b type which
 * specifies C++ type of the corresponding signal or slot argument and
 * @b name which specifies parameter name. Both of this attributes are
 * obligitary.
 *
 * @section code_generation C++ code generation with qrsc utility
 *
 * Once you've described your application remote interface in XML files you can
 * generate classes which you can use in your application. You can do this with
 * @b qrsc application (qrsc is abbriviation for QRemoteSignal compiler).
 *
 * To generate a @e Sevice class (the one to be used in a server application)
 * use the following command:
 * @code
 * qrsc --service --header exampleservice.h --source exampleservice.cpp example.xml
 * @endcode
 * it will create two files @b exampleservice.h and @b exampleservice.cpp
 * with declaration and implementation of class describing your service.
 *
 * To generate a @e Client class (the one to be use in a client application)
 * use the following command:
 * @code
 * qrsc --client --header exampleclient.h --source exampleclient.cpp example.xml
 * @endcode
 * it will create two files @b exampleclient.h and @b exampleclient.cpp with
 * declaration and implementation of class describing interface for calling
 * your service slots and reciving its signals from the client application.
 *
 * You can find more information about @b qrsc in its unix man page or by
 * running
 * @code
 * qrsc --help
 * @endcode
 *
 * @note If you have interface files for the version 0.6.0 of the QRemoteSignal
 * library or older you should rename all @b method elements to @b slot or run
 * @code
 * qrsc --update new_file.xml old_file.xml
 * @endcode
 * to do it automatically. You still can use your old interface with the new
 * version of @b qrsc utility but it will print warning message about outdated
 * format of your service.
 *
 * @section generated_classes Using generated classes in your application
 *
 * Classes generated from XML interface description can be used to call slots
 * and emit signals between different application.
 *
 * First of all you need to register instances of generated class in a
 * ServicesManager class instance (see this class documentation for detales).
 *
 * @subsection generated_service Using generated class in the server
 * application.
 *
 * Here is public signals and slots of the service class generated from the
 * example XML file you can find in the beggining of this page:
 * @code
 * namespace qrs {
 *
 *    class ExampleService: public AbsService {
 *       ...
 *       public slots:
 *          void mySignal(const QString &str);
 *       signals:
 *          void mySlot(const quint32 &num);
 *    };
 *
 * }
 * @endcode
 *
 * Each time you server receives request to call slot @b mySlot instance of
 * this class emits signal @b mySlot you can connect it to any slot in you
 * application to provide meaningfull reaction on this event.
 *
 * Each time you want your server to emit remote signal @b mySignal you need
 * to call @b mySignal slot and you server sends request to the client
 * application to call necessary function.
 *
 * @subsection generated_service Using generated class in the client
 * application.
 *
 * Here is public signals and slots of the client class generated from the
 * example XML file you can find in the beggining of this page:
 * @code
 * namespace qrs {
 *
 *    class ExampleClient: public AbsService {
 *       ...
 *       public slots:
 *          void mySlot(const quint32 &num);
 *       signals:
 *          void mySignal(const QString &str);
 *    };
 *
 * }
 * @endcode
 *
 * Each time you call slot @b mySlot you client application sends to your
 * server application request to call corresponding function.
 *
 * Each time your server application sends signal @b mySignal to you client
 * application signal @b mySignal is emitted. You can connect this signal to
 * some slot in your class to provide meaningfull reaction on this event.
 *
 * @section build_systems How to invoke qrsc from different build systems
 *
 * You can invoke @b qrsc manually each time your XML service description is
 * updated or new one added but it's better to do this from your build system
 * scrits. Here are some tips how to run @b qrsc from different buildsystems.
 *
 * @subsection scons
 *
 * You can find scons tool to invoke @b qrsc from scons in the
 * @e site_scons/site_tools/qrsc.py file of the library distribution. Just use
 * it in you project.
 *
 * @note This tool depends on qt4 tool which you can find it in this library
 * distribution or download from official project repository.
 *
 * This tool provides two builders
 * @li QRSService which creates services classes from XML service descritptions
 * and returns list of C++ sources files.
 * @li QRSClient which creates clients classes from XML service descritptions
 * and returns list of C++ sources files.
 *
 * Here is example how to use them in your SConstruct:
 * @code
 * env = Environment()
 * env.Tool('qt4')
 * env.Tool('qrs')
 *
 * env.EnableQt4Modules(['QtCore'])
 *
 * ServerSources=['server.cpp']
 * ServerSources += env.QRSService('service.xml')
 *
 * ClientSources=['client.cpp']
 * ClientSources += env.QRSClient('service.xml')
 *
 * env.Program('server',ServerSources)
 * env.Program('client',ClientSources)
 * @endcode
 *
 * @subsection qmake
 *
 * This version of QRemoteSignal library comes with qmake custom feature file.
 * You can find it in the library installation directory its name is
 * @b qremotesignal.prf . First of all you need to enshure that the directory
 * containig this file is searched by qmake for configuration features. The
 * simpliest way to do it is to add this directory path to the @b QMAKEFEATURES
 * envirunment variable. After that you can use this feature in you qmake
 * projects. Use @b QRS_SERVICE_INTERFACES variable in you project file to
 * specify which XML file should be used to generate servces classes. Use
 * @b QRS_CLIENT_INTERFACES variable to specify which XML files should be
 * used to generate client classes.
 *
 * Here is example of this feature ussage:
 * @li server.pro
 * @code
 * TEMPLATE = app
 * TARGET = server
 *
 * SOURCES = server.cpp
 * QRS_SERVICE_INTERFACES = service.xml
 *
 * CONFIG += qremotesignal
 * @endcode
 * @li client.pro
 * @code
 * TEMPLATE = app
 * TARGET = client
 *
 * SOURCES = client.cpp
 * QRS_CLIENT_INTERFACES = service.xml
 *
 * CONFIG += qremotesignal
 * @endcode
 */


/**
 * @file QRemoteSignal
 * @brief Public library header file containing all necessary classes
 *
 * Include this header file to automatically include all necessary internall
 * header files.
 */

/**
 * @namespace qrs
 * @brief Main library namespace
 */