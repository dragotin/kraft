/***************************************************************************
              htmlwindow.h  - non modal window showing a html page
                             -------------------
    begin                : Jul 2026
    copyright            : (C) 2026 Klaas Freitag <opensource@freisturz.de>
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HTMLWINDOW_H
#define HTMLWINDOW_H

#include <QWidget>

class HtmlView;

/**
 * A non modal top level window that shows a html page in a HtmlView and
 * offers a Close button in a button box below the view.
 *
 * The window deletes itself when it is closed, so simply create it on the
 * heap and call show().
 */
class HtmlWindow : public QWidget
{
    Q_OBJECT
public:
    // Name must be set to identify the window size in config file
    explicit HtmlWindow(const QString& title, const QString& name, QWidget *parent = nullptr);

    HtmlView *htmlView() const { return _htmlView; }

protected:
    void closeEvent(QCloseEvent *ev);

Q_SIGNALS:
    void closing();

public Q_SLOTS:
    void setHtml(const QString &content);
    void setStylesheetFile(const QString &styleFile);

private:
    HtmlView *_htmlView;
};

#endif // HTMLWINDOW_H
