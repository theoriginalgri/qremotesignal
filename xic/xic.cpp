/**
 * @file xic.cpp
 * @brief Entry point for XML interface compiler
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 29 Jul 2009
 */
#include <stdio.h>

#include <QtCore>
#include <QtXml>
#include <QtXmlPatterns>

#include "interfacedocument.h"
#include "interfacecompiler.h"
#include "xicconfig.h"

#define HELP_FLAG "help"
#define VERSION_FLAG "version"
#define QT_VERSION_FLAG "qt-version"
#define PRINT_FILES_FLAG "print-files"
#define OUT_DIR_OPTION "out-dir"

int main(int argc, char *argv[]) {
   // Application Info
   QCoreApplication app(argc,argv);
   app.setApplicationName("XML interface compiler");
   app.setApplicationVersion("0.1");
   QTextStream out(stdout,QIODevice::WriteOnly);
   QTextStream err(stderr,QIODevice::WriteOnly);
   // Command line options configuration
   XicConfig conf;
   conf.addFlag(HELP_FLAG,conf.tr("Print this help and exit"),'h');
   conf.addFlag(VERSION_FLAG,conf.tr("Print version information and exit"),'v');
   conf.addFlag(QT_VERSION_FLAG,conf.tr("Print Qt version information"));
   conf.addFlag(PRINT_FILES_FLAG,conf.tr("Print output files names and exit"),'f');
   conf.addOption(OUT_DIR_OPTION,conf.tr("Specify output directory"),'d');
   if ( ! conf.parse() ) {
      err << conf.errorMessage() << endl;
      out << conf.helpStr();
      return 1;
   }
   // Print info if requested
   if ( conf.flags()[HELP_FLAG] ) {
      out << conf.helpStr();
      return 0;
   }
   if ( conf.flags()[QT_VERSION_FLAG] ) {
      out << conf.tr("Compiled with Qt: %1").arg(QT_VERSION_STR) << endl;
      out << conf.tr("Running with Qt version: %1").arg(qVersion()) << endl;
   }
   if ( conf.flags()[VERSION_FLAG] ) {
      out << conf.tr("%1 version: %2").arg(app.applicationName(),app.applicationVersion()) << endl;
      return 0;
   }
   if ( conf.arguments().isEmpty() ) {
      err << conf.tr("Error: Input file not specified!") << endl;
      out << conf.helpStr();
      return 1;
   }
   // Compile document
   InterfaceDocument inputDoc( conf.arguments().first() );
   InterfaceCompiler compiler(&inputDoc);

   QString prefix = conf.options()[OUT_DIR_OPTION];
   if ( prefix != QString() ) {
      QFileInfo outDir(prefix);
      if ( ! outDir.isDir() ) {
         err << conf.tr("%1 is not a directory").arg(prefix) << endl;
         return 1;
      }
      inputDoc.setServiceHeader( outDir.filePath() + '/' + inputDoc.serviceHeader() );
      inputDoc.setServiceSource( outDir.filePath() + '/' + inputDoc.serviceSource() );
      inputDoc.setClientHeader( outDir.filePath() + '/' + inputDoc.clientHeader() );
      inputDoc.setClientSource( outDir.filePath() + '/' + inputDoc.clientSource() );
   }

   if ( conf.flags()[PRINT_FILES_FLAG] ) {
      out << inputDoc.serviceHeader() << endl
          << inputDoc.serviceSource() << endl
          << inputDoc.clientHeader() << endl
          << inputDoc.clientSource() << endl;
      return 0;
   }

   if ( !compiler.compileServiceHeader() ) {
      err << app.tr("Failed to compile interface") << endl;
      return 1;
   }
   if ( !compiler.compileServiceSource() ) {
      err << app.tr("Failed to compile interface") << endl;
      return 1;
   }
   if ( !compiler.compileClientHeader() ) {
      err << app.tr("Failed to compile interface") << endl;
      return 1;
   }
   if ( !compiler.compileClientSource() ) {
      err << app.tr("Failed to compile interface") << endl;
      return 1;
   }
   return 0;
}
