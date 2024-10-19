/***************************************************************************
                docidentgenerator - Async doc ident generator
                             -------------------
    begin                : 12/2023
    copyright            : (C) 2023 by Klaas Freitag
    email                : opensource@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "docidentgenerator.h"
#include "doctype.h"
#include "numbercycle.h"
#include "kraftdoc.h"

#include <KLocalizedString>


DocIdentGenerator::DocIdentGenerator(QObject *parent)
    : QObject{parent}
{

}


bool DocIdentGenerator::generate(KraftDoc *doc)
{
    if (!doc) {
        _error = i18n("Ident Generator: Called with empty doc object");
        return false;
    }

    DocType dt = doc->docType();
    const QString ncName = dt.numberCycleName();

    const QString ident = NumberCycles::generateIdent(ncName, dt.name(), doc->date(), doc->addressUid());

    Q_EMIT newIdent(ident);
    return true;
}

QString DocIdentGenerator::errorStr() const
{
    return _error;
}
