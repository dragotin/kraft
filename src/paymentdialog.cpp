/***************************************************************************
             Paymentdialog - Edit payment information
                             -------------------
    begin                : january 2014
    copyright            : (C) 2014 by Klaas Freitag
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

#include <kdialog.h>
#include <klocale.h>

#include "archdoc.h"
#include "dbids.h"

#include "paymentdialog.h"

PaymentDialog::PaymentDialog(QWidget *parent, const dbID& id, qlonglong expectedAmount)
    :QDialog(parent),
    _ui(new  Ui::PaymentDialog)
{
    _archDoc.reset(new ArchDoc(id));

    _ui->setupUi(this);

    setWindowTitle( i18n("Payment") );
    setModal( true );

    setDocInfo(expectedAmount);
}

void PaymentDialog::setDocInfo(qlonglong expectedAmount)
{
    QString docDate = _archDoc->locale()->formatDate(_archDoc->date());
    QString docInfo = i18n("%1 from %2").arg(_archDoc->docType()).arg(docDate);

    _ui->documentInfo->setText(docInfo);

    QDate d = QDate::currentDate();
    if(_archDoc->hasDocState(ArchDoc::Payed)) {
        Geld g = _archDoc->toDigest().payment();
        _ui->amountEdit->setText( g.toString(_archDoc->locale() ));
        d = _archDoc->toDigest().paymentDate();
    } else {
        // FIXME pre-set the amount of expected payment
        Geld g(expectedAmount);
        _ui->amountEdit->setText( g.toString(_archDoc->locale() ));
    }
    _ui->dateEdit->setDate( d );
}

bool PaymentDialog::paymentExpected()
{
    return _ui->paymentBox->isChecked();
}

QDate PaymentDialog::paymentDate()
{
    return _ui->dateEdit->date();
}

Geld PaymentDialog::amount()
{
    QString str = _ui->amountEdit->text();

    Geld g;
    g.fromString(str);

    return g;
}

PaymentDialog::~PaymentDialog()
{

}

