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
#ifndef PAYMENTDIALOG_H
#define PAYMENTDIALOG_H

#include <QWidget>
#include <QScopedPointer>

#include <QDialog>

#include "ui_payment.h"

class ArchDoc;
class Geld;

struct Payment
{
public:
    bool  _expected;
    QDate _date;
    Geld  _amount;
};

class PaymentDialog : public QDialog
{
public:
    PaymentDialog(QWidget *parent, const dbID& docId, qlonglong expectedAmount);
    ~PaymentDialog();

    void setDocInfo(qlonglong expectedAmount);

    bool paymentExpected();
    QDate paymentDate();
    Geld amount();

private:
    QScopedPointer<ArchDoc> _archDoc;
    Ui::PaymentDialog* const _ui;
};

#endif // PAYMENTDIALOG_H
