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
#include <QWebView>

HtmlView::HtmlView( QWidget *parent )
    : QWebView( parent ), mZoomStep( 10 )
{

}

void HtmlView::clearView()
{
    setHtml(QString::null);
    setTitle( QString::null );
}

void HtmlView::setInternalUrl( const QString &url )
{
    mInternalUrl = url;
}

QString HtmlView::internalUrl() const
{
    return mInternalUrl;
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
    bool ok = fi.exists();
    qDebug () << "found this stylefile: " << mStyleSheetFile << ok;
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

QString HtmlView::locateCSSImages( const QByteArray& line )
{
    QString l = QString::fromUtf8(line);

    QRegExp reg( "\\{\\{CSS_IMG_PATH\\}\\}/(\\S+)\\s*\\);");
    if( l.contains(reg) ) {
        QString fName = reg.cap(1);
        QString p;

        if( !fName.isEmpty() ) {
            QByteArray kraftHome = qgetenv("KRAFT_HOME");
            if( !kraftHome.isEmpty() ) {
                p = QString("%1/src/pics/%2").arg(QString::fromUtf8(kraftHome)).arg(fName);
            } else {
                QString find;
                find = QString("kraft/pics/%1").arg(fName);
                p = QStandardPaths::locate( QStandardPaths::AppDataLocation, find );
                if( p.isEmpty() ) {
                    // qDebug () << "ERR: Unable to find resource " << fName;
                }
            }
        }
        if( !p.isEmpty() ) {
            p += ");";
            l.replace(reg, p);
        }
    }
    return l;
}

QString HtmlView::topFrame( )
{
    QString t = QString( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">"
                         "<html><head><title>%1</title>" ).arg( mTitle );

    if ( ! mStyleSheetFile.isEmpty() ) {
        QString style;
        QFile file(mStyleSheetFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            style = QLatin1String("<style type=\"text/css\">");
            while (!file.atEnd()) {
                QString line = locateCSSImages(file.readLine());
                style += line;
            }
            file.close();
            style += QLatin1String("</style>");
        }
        t += style;
    }

    return t;
}

QString HtmlView::bottomFrame()
{
    QString t = "</body>";

    return t;
}

void HtmlView::displayContent( const QString& content )
{
    // qDebug() << "BASE URL: " << mBaseUrl.pretyUrl();
    // qDebug() << "Stylesheet URL: " << mStyleSheetFile;

    // qDebug() << "Show content: " << content;
    setHtml( topFrame() + content + bottomFrame(), mBaseUrl );
}

void HtmlView::setBaseUrl( const QString& base )
{

    mBaseUrl = QUrl( base );
    // qDebug () << "Setting base url: " << mBaseUrl.prettyUrl();
}
