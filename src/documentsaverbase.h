/***************************************************************************
             documentsaverbase  - Base class of a document save class
                             -------------------
    begin                : 2006-02-21
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

#ifndef _DOCUMENTSAVERBASE_H
#define _DOCUMENTSAVERBASE_H

// include files
#include <qobject.h>

/**
 *
 */
class KraftDoc;
class dbID;

class DocumentSaverBase : public QObject
{
  Q_OBJECT

public:
  DocumentSaverBase();
  virtual ~DocumentSaverBase();

  virtual bool saveDocument( KraftDoc* ) = 0;
  virtual void load( const QString&, KraftDoc * ) = 0;
};

#endif

/* END */

