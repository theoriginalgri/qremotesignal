/**
 * @file customtypestests.cpp
 * @brief QRemoteSignal library custom types support tests
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 16 Sep 2009
 */
#include <QtCore/QObject>
#include <QtTest/QTest>

#include "QRemoteSignal"

#include "customstruct.h"

#include "customtypeservice.h"
#include "customtypeclient.h"

class CustomTypesTests: public QObject {
   Q_OBJECT
   public slots:
      void receive(const CustomStruct &val) {
         mLastReceived.mInt = val.mInt;
         mLastReceived.mString = val.mString;
      }

      void receiveList(const QList<CustomStruct> &val) {
          mLastReceivedList = val;
      }

      void receiveMap(const QMap<QString,CustomStruct> &val) {
          mLastReceivedMap = val;
      }
   private slots:
      /// Prepare test environment
      void initTestCase() {
         mServerManager = new qrs::ServicesManager(this);
         mClientManager = new qrs::ServicesManager(this);
         connect(mServerManager,SIGNAL(send(QByteArray)),
                 mClientManager,SLOT(receive(const QByteArray&)));
         connect(mClientManager,SIGNAL(send(QByteArray)),
                 mServerManager,SLOT(receive(const QByteArray&)));
         mService = new qrs::CustomTypeService(mServerManager);
         mClient = new qrs::CustomTypeClient(mClientManager);

         qRegisterMetaType<CustomStruct>("CustomStruct");
         qRegisterMetaType< QList<CustomStruct> >("QList<CustomStruct>");
         qRegisterMetaType< QMap<QString,CustomStruct> >("QMap<QString,CustomStruct>");
      }
      /// Cleanup test environment
      void cleanupTestCase() {
      }

      void sendCustomStructTest_data() {
         QTest::addColumn<int>("mInt");
         QTest::addColumn<QString>("mString");

         QTest::newRow("1") << 1 << "one";
         QTest::newRow("2") << 2 << "two";
         QTest::newRow("3") << 1 << "three";
      }
      void sendCustomStructTest() {
         QSignalSpy spy( mService,SIGNAL(sendCustomStruct(CustomStruct)) );
         connect(mService,SIGNAL(sendCustomStruct(CustomStruct)),
                 this,SLOT(receive(const CustomStruct&)));
         QFETCH(int, mInt);
         QFETCH(QString, mString);

         CustomStruct sent;
         sent.mInt = mInt;
         sent.mString = mString;
         mClient->sendCustomStruct(sent);

         QCOMPARE( spy.count(), 1 );

         QCOMPARE(mLastReceived.mInt , sent.mInt);
         QCOMPARE(mLastReceived.mString , sent.mString);
      }

      void sendListCustomStructTest() {
          QSignalSpy spy( mService,SIGNAL(sendList(QList<CustomStruct>)) );
          connect(mService,SIGNAL(sendList(QList<CustomStruct>)),
                  this,SLOT(receiveList(QList<CustomStruct>)));

          QList<CustomStruct> sent;
          CustomStruct val;
          val.mInt = 1;
          val.mString = "one";
          sent.append(val);
          val.mInt = 2;
          val.mString = "two";
          sent.append(val);
          mClient->sendList(sent);

          QCOMPARE( spy.count(), 1 );

          QCOMPARE(mLastReceivedList.size() , sent.size());
          for(int i = 0; i < mLastReceivedList.size(); i++) {
              QCOMPARE(mLastReceivedList[i].mInt, sent[i].mInt);
              QCOMPARE(mLastReceivedList[i].mString, sent[i].mString);
          }
      }

      void sendMapCustomStructTest() {
          QSignalSpy spy( mService,SIGNAL(sendMap(QMap<QString,CustomStruct>)) );
          connect(mService,SIGNAL(sendMap(QMap<QString,CustomStruct>)),
                  this,SLOT(receiveMap(QMap<QString,CustomStruct>)));

          QMap<QString,CustomStruct> sent;
          CustomStruct val;
          val.mInt = 1;
          val.mString = "one";
          sent["1"] = val;
          val.mInt = 2;
          val.mString = "two";
          sent["2"] = val;
          mClient->sendMap(sent);

          QCOMPARE( spy.count(), 1 );

          QCOMPARE(mLastReceivedMap.keys().size(), sent.keys().size());
          for(int i = 0; i < mLastReceivedMap.keys().size(); i++) {
              QString key = mLastReceivedMap.keys()[i];
              QVERIFY(sent.contains(key));
              QCOMPARE(mLastReceivedMap[key].mInt, sent[key].mInt);
              QCOMPARE(mLastReceivedMap[key].mString, sent[key].mString);
          }
      }

   private:
      qrs::ServicesManager *mServerManager,*mClientManager;
      qrs::CustomTypeService *mService;
      qrs::CustomTypeClient *mClient;

      CustomStruct mLastReceived;
      QList<CustomStruct> mLastReceivedList;
      QMap<QString,CustomStruct> mLastReceivedMap;
};

#include "customtypestests.moc"

QTEST_MAIN(CustomTypesTests);
