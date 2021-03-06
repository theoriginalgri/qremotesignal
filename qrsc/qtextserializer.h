/**
 * @file qtextserializer.h
 * @brief Plain text serializer of QQuery processor
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 29 Jul 2009
 */
#ifndef QTextSerializer_H
#define QTextSerializer_H

#include <QtCore/QTextCodec>
#include <QtCore/QIODevice>
#include <QtCore/QString>
#include <QtCore/QStringRef>
#include <QtCore/QVariant>

#include <QtXmlPatterns/QAbstractXmlReceiver>
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QXmlName>

class QTextSerializer : public QAbstractXmlReceiver {
   public:
      QTextSerializer (const QXmlQuery& query, QIODevice* outputDevice);
      virtual ~QTextSerializer() {};

      const QTextCodec * codec () const {return mTxtCodec;};
      QIODevice* outputDevice () const;
      void setCodec ( const QTextCodec* val ) {mTxtCodec = val;};

      virtual void startElement(const QXmlName &name);
      virtual void endElement();
      virtual void attribute(const QXmlName &name,
                             const QStringRef &value);
      virtual void comment(const QString &value);
      virtual void characters(const QStringRef &value);
      virtual void startDocument();
      virtual void endDocument();

      virtual void processingInstruction(const QXmlName &target,
                                         const QString &value);

      virtual void atomicValue(const QVariant &value);
      virtual void namespaceBinding(const QXmlName &name);
      virtual void startOfSequence();
      virtual void endOfSequence();
   private:
      QIODevice* mOut;
      const QTextCodec* mTxtCodec;
};

#endif
