/***************************************************************************
                        htmlview.cpp  - show a html page
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
#include "htmlview.h"

#include <QFileInfo>
#include <QAction>
#include <QStandardPaths>
#include <QDebug>
#include <QFile>
#include <QWebEnginePage>


HtmlView::HtmlView( QWidget *parent )
    : QWebEngineView( parent ), mZoomStep( 10 )
{
    _webPage.reset(new UrlEmitWebEnginePage);
    setPage( _webPage.data() );
    connect(_webPage.data(), SIGNAL(openUrl(const QUrl&)), this, SIGNAL(openUrl(const QUrl&)));
}

void HtmlView::setTitle( const QString &title )
{
    mTitle = title;
}

void HtmlView::setStylesheetFile( const QString &style )
{
    QString prjPath = QString::fromUtf8(qgetenv( "KRAFT_HOME" ));
    if( !prjPath.isEmpty() ) {
        mStyleSheetFile = QString( "%1/styles/%2" ).arg( prjPath ).arg( style );
    } else {
        mStyleSheetFile = QStandardPaths::locate( QStandardPaths::DataLocation, style );
    }
    QFileInfo fi(mStyleSheetFile);

    if( fi.exists() ) {
        qDebug () << "Found this stylefile: " << mStyleSheetFile;
        setBaseUrl(fi.path()+QLatin1String("/pics/"));
    } else {
        qDebug() << "Unable to find stylesheet file "<< style;
    }
}

void HtmlView::zoomIn()
{
    setZoomFactor( zoomFactor() + mZoomStep );
    updateZoomActions();
}

void HtmlView::zoomOut()
{
    setZoomFactor( zoomFactor() - mZoomStep );
    updateZoomActions();
}

void HtmlView::updateZoomActions()
{
    mZoomInAction->setEnabled( zoomFactor() + mZoomStep <= 300 );
    mZoomOutAction->setEnabled( zoomFactor() - mZoomStep > 100 );

    // Prefs::self()->setZoomFactor( zoomFactor() );
}

QString HtmlView::topFrame( ) const
{
    QString t( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">");
    t += QString("<html><head><title>%1</title>").arg( mTitle );

    if ( ! mStyleSheetFile.isEmpty() ) {
        QString style;
        QFile file(mStyleSheetFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            style = QLatin1String("<style type=\"text/css\">");
            while (!file.atEnd()) {
                const QString l =  QString::fromUtf8(file.readLine());
                style += l;
                // qDebug() << "*******" << l;
            }
            style += QLatin1String("</style>");
            file.close();
        }
        t += style;
    }

    return t;
}

QString HtmlView::bottomFrame() const
{
    const QString t("</body>");

    return t;
}

void HtmlView::displayContent( const QString& content )
{
    const QString out = topFrame() + content + bottomFrame();
    // qDebug() << "OOOOOOOOOOOOOOOOOOOOOO " << out;
    setHtml( out , mBaseUrl );
}

void HtmlView::setBaseUrl( const QString& base )
{

    mBaseUrl = QUrl::fromLocalFile(base);
    // qDebug () << "Setting base url: " << mBaseUrl.prettyUrl();
}
