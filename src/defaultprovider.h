/***************************************************************************
                 defaultprovider.h  - Defaults for this and that
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
#ifndef DEFAULTPROVIDER_H
#define DEFAULTPROVIDER_H

#include <qdom.h>

#include "dbids.h"
#include "kraftdoc.h"

class dbID;
class QStringList;

/**
 * encapsulates all relevant for default values for documents such as
 * texts etc.
 */
class DefaultProvider
{
  public:
    ~DefaultProvider();

    static DefaultProvider *self();
    QStringList docTypes();

    QString documentText( const QString&, const QString&, DocGuardedPtr = 0 );
    void saveDocumentText( const QString&, const QString&, const QString& );

    QString docType(); // the default document type for new docs
  private:
    DefaultProvider();

    static DefaultProvider *mSelf;
};

#endif
