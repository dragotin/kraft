 /***************************************************************************
                        htmlview.h  - show a html page
                             -------------------
    begin                : Aug 2006
    copyright            : (C) 2006 Klaas Freitag <freitag@kde.org>
                           (C) 2006 Cornelius Schumacher <schumacher@kde.org>
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <QWebEngineView>
#include <QDesktopServices>

class QAction;
class QUrl;

class UrlEmitWebEnginePage : public QWebEnginePage
{
    Q_OBJECT

signals:
    void openUrl( const QUrl& );

protected:

    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
    {
        Q_UNUSED(type);
        Q_UNUSED(isMainFrame);
        if( url.scheme().startsWith("http") && url.host() != "localhost" ) {
            // open normal pages.
            QDesktopServices::openUrl(url);
            return false;
        }

        const QString urlStr = url.toString();
        qDebug() << "openUrl hit:" << urlStr;
        emit openUrl(url);
        return false;
    }
};

class HtmlView : public QWebEngineView
{
    Q_OBJECT
  public:
    HtmlView( QWidget *parent = 0);

    QString title() const { return mTitle; }

    void setBaseUrl( const QString& );

  public slots:
    void setTitle( const QString & );
    void setStylesheetFile( const QString & );
    void displayContent( const QString& );

    void zoomIn();
    void zoomOut();

  protected:
    virtual QString topFrame() const;
    virtual QString bottomFrame() const;

    void updateZoomActions();

  signals:
    void openUrl( const QUrl& );

  private:
    QString mTitle;
    QString mInternalUrl;
    QString mStyleSheetFile;

    QAction *mZoomInAction;
    QAction *mZoomOutAction;
    QUrl     mBaseUrl;

    int mZoomStep;

    QScopedPointer<UrlEmitWebEnginePage> _webPage;
};

#endif
