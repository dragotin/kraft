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


HtmlView::HtmlView( QWidget *parent )
    : QTextBrowser( parent ), mZoomStep( 10 )
{
    this->setReadOnly(true);
    this->setOpenLinks(false); // get only the signal below
    connect (this, SIGNAL(anchorClicked(QUrl)), this, SIGNAL(openUrl(QUrl)));
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
        mStyleSheetFile = QStandardPaths::locate( QStandardPaths::DataLocation, QString("styles/%1").arg(style));
    }
    QFileInfo fi(mStyleSheetFile);

    if( fi.exists() ) {
        qDebug () << "Found this stylefile: " << mStyleSheetFile;
        // Important: Append the slash here, otherwise the base url is wrong.
        setBaseUrl(fi.path()+"/");
    } else {
        qDebug() << "Unable to find stylesheet file "<< style;
    }
}

void HtmlView::zoomIn()
{
    // setZoomFactor( zoomFactor() + mZoomStep );
    updateZoomActions();
}

void HtmlView::zoomOut()
{
    // setZoomFactor( zoomFactor() - mZoomStep );
    updateZoomActions();
}

void HtmlView::updateZoomActions()
{
   // mZoomInAction->setEnabled( zoomFactor() + mZoomStep <= 300 );
   // mZoomOutAction->setEnabled( zoomFactor() - mZoomStep > 100 );

    // Prefs::self()->setZoomFactor( zoomFactor() );
}

QString HtmlView::topFrame( ) const
{
    QStringList stringList;
    stringList << QLatin1String("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\">");
    stringList << QLatin1String("<html>");

    if(!mTitle.isEmpty()) {
        stringList << QString("<head><title>%1</title></head>").arg( mTitle );
    }
    stringList << QLatin1String("<body>");
    return stringList.join(QChar('\n'));
}

QString HtmlView::styles() const
{
    QFile file(mStyleSheetFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textStream(&file);
        QString styles = textStream.readAll();
        file.close();

        // replace the relative urls of images with absolute pathes
        QRegExp rx("(image:\\s*url\\()(.+)\\)");
        rx.setMinimal(true);
        int pos;
        while ((pos = rx.indexIn(styles, pos)) != -1) {
            QString m = rx.cap(2);
            if( m.startsWith("/")) {
                // absolute path, all fine
            } else {
                int oldLen = m.length();
                m.prepend( mBase);
                int newLength = m.length();
                int standingLen = rx.cap(1).length();
                styles.replace(pos + standingLen, oldLen, m);
                pos += standingLen+newLength;
            }
            // qDebug() << "+++++++++++++++++++++++++++++++++++++++++++++" << m;
            pos += rx.matchedLength();
        }
        return styles;
    }
    return QString::null;
}

QString HtmlView::bottomFrame() const
{
    const QString t("</body>\n"
                    "</html>");

    return t;
}

void HtmlView::displayContent( const QString& content )
{
    // on empty content just clear and leave.
    if( content.isEmpty() ) {
        this->clear();
        return;
    }

    const QString out = topFrame() + content + bottomFrame();
    const QString s = styles();

#ifdef QT_DEBUG
    qDebug() << "########## HtmlView output written to /tmp/kraft.html";
    QFile caFile("/tmp/kraft.html");
    caFile.open(QIODevice::WriteOnly | QIODevice::Text);

    if(!caFile.isOpen()){
        qDebug() << "- Error, unable to open" << "outputFilename" << "for output";
    }
    QTextStream outStream(&caFile);
    outStream << s;
    outStream << "##############" << endl;
    outStream << out;
    caFile.close();
#endif

    this->document()->setDefaultStyleSheet(s);
    setHtml(out);
}

void HtmlView::setBaseUrl( const QString& base )
{

    mBase = base;
    qDebug () << "Setting base url: " << mBase;
}
