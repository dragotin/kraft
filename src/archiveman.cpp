/***************************************************************************
                       archiveman.cpp  - Archive Manager
                             -------------------
    begin                : July 2006
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
#include <kstaticdeleter.h>
#include <kdebug.h>

#include "archiveman.h"
#include "kraftdoc.h"

static KStaticDeleter<ArchiveMan> selfDeleter;

ArchiveMan* ArchiveMan::mSelf = 0;

ArchiveMan *ArchiveMan::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new ArchiveMan() );
  }
  return mSelf;
}

ArchiveMan::ArchiveMan()
{

}

ArchiveMan::~ArchiveMan()
{

}

QString ArchiveMan::archiveDocument( KraftDoc *doc )
{
  QString res; 
  if( ! doc ) return res;
    
  QDomDocument xmldoc( "kraftdocument" );
  QDomElement root = xmldoc.createElement( "kraftdocument" );
  xmldoc.appendChild( root );
  QDomElement cust = xmldoc.createElement( "client" );
  root.appendChild( cust );
  cust.appendChild( xmlTextElement( xmldoc, "address", doc->address() ) );
  cust.appendChild( xmlTextElement( xmldoc, "clientId", doc->addressUid() ) );
  
  QDomElement docElem = xmldoc.createElement( "docframe" );
  root.appendChild( docElem );
  docElem.appendChild( xmlTextElement( xmldoc, "docType", doc->docType() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "ident", doc->ident() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "preText", doc->preText() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "postText", doc->postText() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "salut", doc->salut() ) );
  docElem.appendChild( xmlTextElement( xmldoc, "goodbye", doc->goodbye() ) );
  
  docElem.appendChild( xmlTextElement( xmldoc, "date", 
                    KGlobal().locale()->formatDate( doc->date() ) ) );
  
  root.appendChild( doc->positions().domElement( xmldoc ) );
  
  QString xml = xmldoc.toString();
  kdDebug() << "Resulting XML: " << xml << endl;
  
  return res;
}

QDomElement ArchiveMan::xmlTextElement( QDomDocument doc, const QString& name, const QString& value )
{
  QDomElement elem = doc.createElement( name );
  QDomText t = doc.createTextNode( value );
  elem.appendChild( t );
  return elem;
}

