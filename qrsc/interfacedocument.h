/**
 * @file interfacedocument.h
 * @brief InterfaceDocument class
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 4 Aug 2009
 */
#ifndef InterfaceDocument_H
#define InterfaceDocument_H

#include <QtCore>

class InterfaceDocument {
   public:
      InterfaceDocument(const QString& path);
      ~InterfaceDocument() {};

      QIODevice* getIODevice() {return &mInterfaseFile;};
      const QString& name() const {return mName;};

      const QString& serviceHeader() const {return mServiceHeader;};
      void setServiceHeader(const QString& val) {mServiceHeader = val;};

      const QString& serviceSource() const {return mServiceSource;};
      void setServiceSource(const QString& val) {mServiceSource = val;};

      const QString& clientHeader() const {return mClientHeader;}
      void setClientHeader(const QString& val) {mClientHeader = val;}

      const QString& clientSource() const {return mClientSource;}
      void setClientSource(const QString& val) {mClientSource = val;}

      bool isValid() const {return mValid;}
      const QString& error() const {return mError;};
   private:
      QFile mInterfaseFile;
      QString mName;
      QString mServiceHeader,mServiceSource;
      QString mClientHeader,mClientSource;
      bool mValid;
      QString mError;
};

#endif
