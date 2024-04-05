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

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QWidget>

class HtmlView;

class DashBoard : public QWidget
{
    Q_OBJECT
public:
    explicit DashBoard(QWidget *parent = nullptr);

    void appendHtml(const QString& t);

signals:

private:
    HtmlView *_htmlView;

};

#endif // DASHBOARD_H
