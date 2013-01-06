/***************************************************************************
             xmldocument - XML document writer, kode based.
                             -------------------
    begin                : 2013-02-15
    copyright            : (C) 2013 by Klaas Freitag
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

#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

#include "kode/document.h"

class KraftDoc;

using namespace KraftXml;

class XmlDocument : public Kraftdocument
{
public:
    XmlDocument();
    // write the KraftDoc from the pointer
    void setKraftDoc( KraftDoc *doc );

    // read the KraftDoc into the pointer
    void getKraftDoc( KraftDoc* );

};

#endif // XMLDOCUMENT_H
