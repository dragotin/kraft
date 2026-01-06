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

#include <QDebug>
#include <QString>
#include <QByteArray>

#include "epcqrcode.h"
#include "geld.h"
#include <3rdparty/qrcodegen.hpp>

#include <iostream>
#include <sstream>

using namespace qrcodegen;

/*
 * Based on this code: https://www.nayuki.io/page/qr-code-generator-library
 */

namespace {
// Returns a string of SVG code for an image depicting the given QR Code, with the given number
// of border modules. The string always uses Unix newlines (\n), regardless of the platform.
std::string toSvgString(const QrCode &qr, int border) {
    if (border < 0)
        throw std::domain_error("Border must be non-negative");
    if (border > INT_MAX / 2 || border * 2 > INT_MAX - qr.getSize())
        throw std::overflow_error("Border too large");

    std::ostringstream sb;
    sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
    sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ";
    sb << (qr.getSize() + border * 2) << " " << (qr.getSize() + border * 2) << "\" stroke=\"none\">\n";
    sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
    sb << "\t<path d=\"";
    for (int y = 0; y < qr.getSize(); y++) {
        for (int x = 0; x < qr.getSize(); x++) {
            if (qr.getModule(x, y)) {
                if (x != 0 || y != 0)
                    sb << " ";
                sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
            }
        }
    }
    sb << "\" fill=\"#000000\"/>\n";
    sb << "</svg>\n";
    return sb.str();
}
}

EPCQRCode::EPCQRCode()
{

}

QByteArray EPCQRCode::asText(const Geld& g,
                             const QString& bacName,
                             const QString& bacBIC,
                             const QString& bacIBAN,
                             const QString& reason)
{
    QByteArray re;

    double sum = g.toDouble();

    if ( bacName.isEmpty() || bacIBAN.isEmpty() || sum < 0.1) {
        qDebug() << "Unable to generate EPC Code - insufficient bank account data.";
        return re;
    }

    re.append("BCD\n"
            "002\n"
            "1\n"
            "SCT\n");
    re.append(bacBIC.toUtf8());
    re.append("\n");

    re.append(bacName.toUtf8());
    re.append("\n");

    re.append(bacIBAN.toUtf8());
    re.append("\n");

    const QString money = QString("EUR%1").arg(QString::number(sum, 'f', 2));
    re.append(money.toUtf8());
    re.append("\n\n");
    re.append(reason.toUtf8());

    return re;
}

QString EPCQRCode::asSvg(const Geld& g, const QString &bacName, const QString &bacBIC, const QString &bacIBAN, const QString &reason)
{
    int border = 2;
    QString svg;
    QByteArray arr = asText(g, bacName, bacBIC, bacIBAN, reason);
    if (!arr.isEmpty()) {
        QrCode qr0 = QrCode::encodeText(arr.data(), QrCode::Ecc::MEDIUM);

        svg = QString::fromStdString(toSvgString(qr0, border));
    }

    return svg;
}
