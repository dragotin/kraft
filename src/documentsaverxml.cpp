/***************************************************************************
             templatesaverbase  -
                             -------------------
    begin                : 2005-20-01
    copyright            : (C) 2005 by Klaas Freitag
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

// include files for KDE
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <QDir>
#include <QSqlQuery>

#include "documentsaverxml.h"
#include "docposition.h"
#include "kraftdoc.h"
#include "kraftdb.h"
#include "unitmanager.h"
#include "dbids.h"
#include "kraftsettings.h"
#include "doctype.h"
#include "xmldocument.h"
/*
 * Use the kxml_compiler generated XML classes, covered by xmldocument class.
 */
using namespace KraftXml;

DocumentSaverXML::DocumentSaverXML( ) : DocumentSaverBase(),
                                      PosTypePosition( QString::fromLatin1( "Position" ) ),
                                      PosTypeExtraDiscount( QString::fromLatin1( "ExtraDiscount" ) ),
                                      PosTypeHeader( QString::fromLatin1( "Header" ) )
{

}

QString DocumentSaverXML::storagePath()
{
    QString path;
    KStandardDirs stdDirs;

    path = KraftSettings::self()->xmlDocPath();

    if(path.isEmpty()) {
        path = stdDirs.saveLocation( "data", "kraft/documents", true );
    }
    if ( ! path.endsWith( QLatin1Char('/'))) path.append(QLatin1Char('/'));

    QDir dir(path);
    if( !dir.exists() ) {
        dir.mkpath(path);
    }
    if( !dir.exists() ) {
        kDebug() << "ERR: No document save path!";
        path = QLatin1String("/tmp/"); // best compromise...
    }

    return path;
}

// The incoming id is a database id from the documents table.
// It needs to be converted to the ident.
QString DocumentSaverXML::loadFileName( const QString& dbId )
{
    if( dbId.isEmpty() ) return QString::null;

    QSqlQuery query("SELECT ident FROM document where docID="+dbId);
    QString ident;

    while (query.next()) {
         ident = query.value(0).toString();
    }

    if( ident.isEmpty() ) {
        qDebug() << "Could not retrieve doc ident from database cache!";
        return QString::null;
    }

    QString path = storagePath();

    QString file("kraft-");
    file.append( ident );
    file.append(QLatin1String(".xml"));

    QFileInfo fi;
    fi.setFile(path, file);

    return fi.canonicalFilePath();
}

// finds a not yet used fiel name, does never overwrite an existing file.
QString DocumentSaverXML::saveFileName( const QString& ident )
{
    if( ident.isEmpty() ) return QString::null;

    QString path = storagePath();

    QString file("kraft-");
    file.append( ident );
    file.append(QLatin1String(".xml"));

    // make sure its not yet existing.
    QFileInfo fi;
    int i = 0;

    fi.setFile( path, file );
    while( ++i < 10 ) {
        if( !fi.exists() ) return fi.filePath();
        QString file("kraft-");
        file.append(ident);
        file.append(QLatin1Char('-'));
        file.append(i);
        file.append(QLatin1String(".xml"));
        fi.setFile(path, file);
    }

    return QString::null;
}

bool DocumentSaverXML::saveDocument(KraftDoc *doc )
{
    bool result = false;
    if( ! doc ) return result;

    kDebug() << "############### Document Save XML ################" << endl;
    XmlDocument xmlDoc;
    xmlDoc.setKraftDoc(doc);
    QString fileName = saveFileName(doc->ident());
    kDebug() << "Saving to file name " << fileName;
    xmlDoc.writeFile(fileName);

    return true;
}


void DocumentSaverXML::load( const QString& id, KraftDoc *doc )
{
    QString fileName = loadFileName(id);

    kDebug() << "############### Document Load XML ################" << endl;
    bool ok;
    XmlDocument xmlDoc;
    xmlDoc.parseFile( fileName, &ok );

    if( ok ) {
        xmlDoc.getKraftDoc( doc );
    } else {
        qDebug() << "FATAL: Failed to parse XML document.";
    }
}

DocumentSaverXML::~DocumentSaverXML( )
{

}

/* END */

