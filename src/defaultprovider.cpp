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

#include "defaultprovider.h"
#include "kraftdb.h"
#include "doctext.h"
#include "kraftsettings.h"
#include "doctype.h"
#include "kraftdoc.h"
#include "dbids.h"

#include <klocalizedstring.h>

Q_GLOBAL_STATIC(DefaultProvider, mSelf)

#if 0
// translations for table DocTypes
I18N_NOOP("Acceptance of Order")
I18N_NOOP("Delivery Receipt")
I18N_NOOP("Invoice")
I18N_NOOP("Offer")
#endif

DefaultProvider *DefaultProvider::self()
{
  return mSelf;
}

DefaultProvider::DefaultProvider()
{

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

  // qDebug() << "Reading texts from DB with: " << sql << endl;

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
  return QStandardPaths::findExecutable( "iconv" );
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

// this method uses QStandardPath::locate from the AppDataLocation to find
// files, but if KRAFT_HOME is set, that one is preffered.
QString DefaultProvider::locateFile(const QString& findFile) const
{
    QString re;
    const QString prjPath = QString::fromUtf8(qgetenv( "KRAFT_HOME" ));

    if( prjPath.isEmpty()) {
        re = QStandardPaths::locate( QStandardPaths::AppDataLocation, findFile );
    } else {
        re = prjPath;
        if( !re.endsWith(QChar('/')) ) {
            re.append( QChar('/'));
        }

        re.append(findFile);
        QFileInfo fi(re);
        if( !fi.exists() ) {
            if( findFile.startsWith("pics")) {
                // special handling: formerly the pics in KRAFT_HOME were in src.
                re = prjPath;
                if( !re.endsWith(QChar('/')) ) {
                    re.append( QChar('/'));
                }
                re.append("src/");
                re.append(findFile);
            } else {
                qDebug() << "WARN: locateFile could not find file " << findFile;
            }
        }
    }

    return re;
}

DefaultProvider::~DefaultProvider()
{

}

