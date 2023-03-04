/***************************************************************************
                  defaultprovider.cpp  - Default Providing Class
                             -------------------
    begin                : November 2006
    copyright            : (C) 2006 by Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QIcon>
#include <QStandardPaths>

#include "defaultprovider.h"
#include "kraftdb.h"
#include "doctext.h"
#include "kraftsettings.h"
#include "doctype.h"
#include "kraftdoc.h"
#include "dbids.h"

#include <klocalizedstring.h>

Q_GLOBAL_STATIC(DefaultProvider, mSelf)

DefaultProvider *DefaultProvider::self()
{
  return mSelf;
}

DefaultProvider::DefaultProvider()
{
   QString basePath = KraftSettings::self()->xmlDocumentsBasePath();

   if (basePath.isEmpty()) {
       basePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
       basePath.append("/xmldoc/v1"); // FIXME: version should not be hardcoded!

       QDir d(basePath);
       if (!d.exists()) {
           d.mkpath(basePath);
       }
   }
   if (basePath.isEmpty()) {
       qDebug() << "Unable to find a root dir for the XML storage";
       return;
   }

   KraftSettings::setXmlDocumentsBasePath(basePath);
   _persister.setBasePath(basePath);
}

DocumentSaverBase& DefaultProvider::documentPersister()
{
    return self()->_persister;
}


QIcon DefaultProvider::icon(const QString& name)
{
    const QString fullIconName = QString(":kraft/custom-icons/%1.svg").arg(name);
    const QIcon icon { fullIconName };
    return icon;
}

QString DefaultProvider::docType()
{
  QString type = KraftSettings::self()->doctype();
  if ( type.isEmpty() ) {
    QStringList allTypes = DocType::allLocalised();
    if( ! allTypes.isEmpty() ) {
      type = DocType::allLocalised()[0];
    } else {
      type = i18n( "Unknown" );
    }
  }
  return type;
}

DocTextList DefaultProvider::documentTexts( const QString& docType, KraftDoc::Part tt )
{
  DocTextList re;

  QString typeStr = DocText::textTypeToString( tt );

  QString sql = QString( "SELECT texts.docTextID, texts.name, texts.text, texts.description, "
                         "texts.textType, types.name as docTypeName FROM DocTexts texts, "
                         "DocTypes types WHERE texts.docTypeId=types.docTypeID AND "
                         "types.name=\'%1\' AND textType = \'%2\'").arg( docType ).arg( typeStr );

  // qDebug() << "Reading texts from DB with: " << sql;

  QSqlQuery query( sql );
  if ( query.isActive() ) {
    while ( query.next() ) {
      DocText dt;
      dt.setDbId( query.value( 0 ) /* docTextID */ .toInt() );
      dt.setName( query.value( 1 ) /* name */ .toString() );
      dt.setText( KraftDB::self()->mysqlEuroDecode( query.value( 2 ) /* text */ .toString() ) );
      dt.setDescription( query.value( 3 ) /* description */ .toString() );
      dt.setTextType( DocText::stringToTextType( query.value( 4 ) /* textType */ .toString() ) );
      dt.setDocType( query.value( 5 ) /* docType */ .toString() );

      re.append( dt );
    }
  }
  return re;
}

QString DefaultProvider::defaultText( const QString& docType, KraftDoc::Part p, DocGuardedPtr )
{
  QString re;

  DocTextList list = documentTexts( docType, p );
  DocTextList::iterator it;

  for ( it = list.begin(); it != list.end(); ++it ) {
    if( (*it).isStandardText() ) {
      re = ( *it ).text();
      break;
    }
  }
  return re;
}

dbID DefaultProvider::saveDocumentText( const DocText& t )
{
  dbID retVal;

  QSqlTableModel model;
  model.setTable( "DocTexts" );

  if ( t.dbId().isOk() ) {
    // qDebug () << "Doing update!";
    model.setFilter( "docTextID=" + t.dbId().toString() );
    model.select();

    if( model.rowCount() > 0 ) {
      QSqlRecord record = model.record(0);
      record.setValue( "docTextID", t.dbId().toString() );
      record.setValue( "name", t.name() );
      record.setValue( "description", t.description() );
      record.setValue( "text", KraftDB::self()->mysqlEuroEncode( t.text() ) );
      record.setValue( "docType", t.docType() );
      record.setValue( "docTypeId", DocType::docTypeId( t.docType() ).toString() );
      record.setValue( "textType",  t.textTypeString() );
      model.setRecord(0, record);
      model.submitAll();
    }
  } else {
    // qDebug () << "Doing insert!";
    QSqlRecord record = model.record();
    record.setValue( "name", t.name() );
    record.setValue( "description", t.description() );
    record.setValue( "text", KraftDB::self()->mysqlEuroEncode( t.text() ) );
    record.setValue( "docType", t.docType() );
    record.setValue( "docTypeId", DocType::docTypeId( t.docType() ).toString() );
    record.setValue( "textType",  t.textTypeString() );

    model.insertRecord(-1, record);
    model.submitAll();
  }


  retVal = KraftDB::self()->getLastInsertID();

  return retVal;
}


QLocale* DefaultProvider::locale()
{
  return &_locale;
}

void DefaultProvider::deleteDocumentText( const DocText& dt )
{
  if ( dt.dbId().isOk() ) {
    QSqlQuery q;
    q.prepare("DELETE FROM DocTexts WHERE docTextID=" + dt.dbId().toString() ) ;
    q.exec();
  } else {
    // qDebug () << "Delete document text not ok: " << dt.text();
  }
}

QString DefaultProvider::currencySymbol() const
{
  return self()->locale()->currencySymbol();
}

QString DefaultProvider::iconvTool() const
{
  return locateBinary( "iconv" );
}

QString DefaultProvider::getStyleSheet( const QString& styleName ) const
{
  QString style;
  if( styleName.isEmpty() ) return style;

  const QString findFile = QString("styles/%1.style").arg(styleName);

  const QString tmplFile = locateFile(findFile);

  QFile data( tmplFile );
  if (data.open( QFile::ReadOnly )) {
    QTextStream readIn( &data );
    style = readIn.readAll();
    data.close();
  }
  return style;
}

// this method first checks if KRAFT_HOME is set. If it is it tries to read the files from there.
// If KRAFT_HOME is not set, it uses QStandardPath::locate from the AppDataLocation to find
// files.
//
// For AppImage, this method should actually look relative to the application directory.
//
QString DefaultProvider::locateFile(const QString& findFile) const
{
    QString re;
    const QString kraftHome = QString::fromUtf8(qgetenv( "KRAFT_HOME" ));

    if (!kraftHome.isEmpty()){
        // KRAFT_HOME is set
        QString fifi {kraftHome};
        if (!fifi.endsWith('/') && !findFile.startsWith('/'))
            fifi.append('/');
        fifi.append(findFile);

        if (QFile::exists(fifi)) {
            re = fifi;
        }
    }

    if (re.isEmpty()) {
        // it was not found in the system location or in KRAFT_HOME
        // If so, check relative to the binary for AppImage.
        QString fifi = QString("%1/../share/kraft/%2").arg(QCoreApplication::applicationDirPath()).arg(findFile);

        if (QFile::exists(fifi)) {
            QFileInfo fi(fifi);
            re = fi.absoluteFilePath();
        }
    }

    // check the system paths
    if (re.isEmpty()) {
        // prepend the kraft path segment and look in the system resources
        QString fifi {findFile};
        re = QStandardPaths::locate( QStandardPaths::AppDataLocation, fifi);
    }

    if (re.isEmpty()) {
        qDebug() << "locateFile could not find file " << findFile;
    }

    return re;
}

QStringList DefaultProvider::locatePythonTool(const QString& toolName) const
{
    QString fullPath;

    // first use the standard locateFile to consider KRAFT_HOME and relative...

    fullPath = locateFile("tools/" + toolName);

    // if that is empty, go for the system executables
    if (fullPath.isEmpty()) {
        fullPath = QStandardPaths::findExecutable(toolName);
    }

    QFileInfo fi(fullPath);
    if (!fi.exists()) {
        fullPath.clear();
    }

    // -- check for python.
    // Default is python3
    // If Kraft is running from an AppImage, we rather use the python from conda which is
    // installed in a relative path.
    QString python {"python3"};
    const QString pypath = QCoreApplication::applicationDirPath() + QStringLiteral("/../conda/bin/python");
    QFileInfo fip(pypath);
    if (fip.exists() && fip.isExecutable()) {
        python = fip.canonicalFilePath();
    }

    QStringList rep {python, fullPath};
    qDebug() << "Returning tool path" << rep;

    return rep;
}

QString DefaultProvider::locateBinary(const QString& name) const
{
    // check the current app path and check if the binary is in there. (AppImage)
    const QString path = QCoreApplication::applicationDirPath();
    const QString localPrg = QString("%1/%2").arg(path).arg(name);
    QFileInfo fi{localPrg};

    if (fi.exists() && fi.isExecutable()) {
        qDebug() << "Returning tool path" << fi.absoluteFilePath() << "for" << name;
        return fi.absoluteFilePath();
    }

    const QString bin = QStandardPaths::findExecutable( name );

    return bin;
}

bool DefaultProvider::writeXmlArchive()
{
    return KraftSettings::self()->doXmlArchive();
}

QString DefaultProvider::xmlArchivePath()
{
    return KraftSettings::self()->xmlArchivePath();
}

QString DefaultProvider::pdfOutputDir()
{
    return KraftSettings::self()->pdfOutputDir();
}

DefaultProvider::~DefaultProvider()
{

}

