/***************************************************************************
                           EPC QR Code generator
                           ---------------------
    begin                : August 2022
    copyright            : (C) 2022 by Klaas Freitag
    email                : kraft@freisturz.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EPCQRCODE_H
#define EPCQRCODE_H

#include "kcontacts/addressee.h"

#include <QPixmap>
#include <QSize>

class Geld;

class EPCQRCode
{
public:
    EPCQRCode();

    QByteArray asText(const Geld& g, const QString &bacName, const QString &bacBIC, const QString &bacIBAN, const QString &reason);

    QString asSvg(const Geld& g, const QString &bacName, const QString &bacBIC, const QString &bacIBAN, const QString &reason);

 //   QPixmap asPng(const Geld& g, const QString &bacName, const QString &bacBIC, const QString &bacIBAN, const QString &reason, const QSize& s);

private:
    KContacts::Addressee _contact;

    const QString _App  {"KAddressbook"};
    const QString _IBAN {"IBAN"};
    const QString _BIC  {"BIC"};
};

#endif // EPCQRCODE_H
