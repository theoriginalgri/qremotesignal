/**
 * @file argvparser.cpp
 * @brief ArgvParser class
 *
 * @author VestniK (Sergey N.Vidyuk) sir.vestnik@gmail.com
 * @date 7 Aug 2009
 */
#include "argvparser.h"

#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>

namespace cmd {
const QString help = "help";
const QString version = "version";
const QString qt_version = "qt-version";
}

ArgvParser::ArgvParser(const ArgvConf &conf, QTextStream *out, QTextStream *err):
    mOut(out), mErr(err)
{
    qApp->setApplicationName(tr(conf.name));
    qApp->setApplicationVersion(conf.version);
    mAppDescription = tr(conf.description);
    addFlag(cmd::help, tr("Print this help and exit."),cmd::help[0]);
    addFlag(cmd::version, tr("Print version information and exit."),cmd::version[0]);
    addFlag(cmd::qt_version, tr("Print Qt version information and exit."));
}

void ArgvParser::addFlag(const QString& name,
                         const QString& desc,
                         const QChar& shortName) {
    mFlags.insert(name,false);
    mShortNames.insert(shortName,name);
    mDescriptions.insert(name,"\n\t\t\t"+desc);
}

void ArgvParser::addOption(const QString& name,
                           const QString &paramName,
                           const QString& desc,
                           const QChar& shortName,
                           const QString& defaultVal ) {
    mOptions.insert(name,defaultVal);
    mShortNames.insert(shortName,name);
    mDescriptions.insert(name," "+paramName+"\n\t\t\t"+desc);
}

/**
 * Require QCoreApplication or QApplication to be initialized
 */
bool ArgvParser::parse() {
    QStringList args = qApp->arguments();
    mExecStr = args[0];
    QFileInfo executableInfo(mExecStr);
    mExecutableName = executableInfo.baseName();
    int i = 1; // args[0] is application itself
    while ( i < args.size() ) {
       QString arg = args[i];
       i++;

       QString name;
       if ( arg.startsWith("--") ) {
           name = arg.mid(2);
       } else {
           if ( arg.startsWith("-") ) {
               name = mShortNames[arg.at(1)];
           } else {
               mArguments.append(arg);
               continue;
           }
       }
       if ( mFlags.contains(name) ) {
           mFlags[name] = true;
           continue;
       }
       if ( mOptions.contains(name) ) {
           if ( i >= args.size() ) {
               (*mErr) << tr("Option %1 require a value").arg(arg) << endl;
               printHelp();
               return false;
           }
           if ( args[i].startsWith('-') ) {
               (*mErr) << tr("Option %1 require a value").arg(arg) << endl;
               printHelp();
               return false;
           }
           mOptions[name] = args[i];
           i++;
           continue;
       }
       (*mErr) << tr("Unknown option: %1").arg(arg) << endl;
       printHelp();
       return false;
    }
    return true;
}

bool ArgvParser::handleHelp()
{
    if (mFlags[cmd::help]) {
        printHelp();
        return true;
    }
    if (mFlags[cmd::qt_version]) {
        printQtVersion();
        return true;
    }
    if (mFlags[cmd::version]) {
        printVersion();
        return true;
    }
    return false;
}

void ArgvParser::printHelp() const {
    QTextStream &out = *mOut;
    for (int i = 0; i < mUsageDescriptions.count(); i++) {
        if ( i == 0 ) {
            out << tr("Usage: ");
        } else {
            out << tr("  or:  ");
        } 
        out << mExecutableName << " " << mUsageDescriptions[i] << endl;
    }
    out << endl;
    if ( ! mAppDescription.isEmpty() ) {
        out << mAppDescription << endl;
        out << endl;
    }
    out << tr("Options:") << endl;
    foreach ( QString name, mDescriptions.keys() ) {
        out << "\t";
        QChar shortName = mShortNames.key(name);
        if ( shortName != QChar() ) {
            out << "-" << shortName << ", ";
        }
        out << "--" << name;
        out << mDescriptions[name] << endl;
    }
    out << endl;
}

void ArgvParser::printQtVersion() const {
    QTextStream &out = *mOut;
    out << tr("Compiled with Qt: %1").arg(QT_VERSION_STR) << endl;
    out << tr("Running with Qt version: %1").arg(qVersion()) << endl;
}

void ArgvParser::printVersion() const {
    QTextStream &out = *mOut;
    out << mExecutableName << " ("
        << qApp->applicationName()
        << ") " << qApp->applicationVersion()
        << endl;
}
