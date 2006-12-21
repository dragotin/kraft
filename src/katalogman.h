/***************************************************************************
             katalogman  -
                             -------------------
    begin                : 2004-12-09
    copyright            : (C) 2004 by Klaas Freitag
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

#ifndef _KATALOGMAN_H
#define _KATALOGMAN_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdict.h>

#include "katalog.h"
// include files

/**
 *
 */
class QStringList;


class KatalogMan : public QObject
{
public:
    ~KatalogMan();
    static KatalogMan *self();

    QStringList allKatalogNames();
    Katalog* getKatalog(const QString&);
    void registerKatalog( Katalog* );
    QString catalogTypeString( const QString& catName );
private:
    KatalogMan();
    static KatalogMan *mSelf;

    QDict<Katalog> m_katalogDict;
};

#endif

/* END */

