/***************************************************************************
                Dashboard - Home Dashboard of Kraft
                             -------------------
    begin                : Mar. 2024
    copyright            : (C) 2024 by Klaas Freitag
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

#include <QVBoxLayout>

#include <klocalizedstring.h>

#include "version.h"
#include "htmlview.h"
#include "dashboard.h"

DashBoard::DashBoard(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *layout = new QVBoxLayout;

    _htmlView = new HtmlView(this);
    const QString h = i18n("<h1>Welcome to Kraft!</h1><small>Kraft version %1</small>", Kraft::Version::number());
    _htmlView->displayContent(h);
    layout->addWidget(_htmlView);
    setLayout(layout);
}

void DashBoard::appendHtml(const QString& t)
{
    QString current = _htmlView->toHtml();

    _htmlView->setHtml(current + t);
}
