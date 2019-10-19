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

#include "defaultprovider.h"


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
    const QString file = QString("styles/%1").arg(style);
    const QString stylesheetFile = DefaultProvider::self()->locateFile(file);

    if( QFile::exists(stylesheetFile) ) {
        qDebug () << "Found this stylefile: " << stylesheetFile;
        mStyles = readStyles(stylesheetFile);
    } else {
        qDebug() << "Unable to find stylesheet file "<< style;
        mStyles.clear();
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

QString HtmlView::readStyles(const QString& styleFile) const
{
    QFile file(styleFile);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styles;
        QFileInfo fi(styleFile);
        const QString base = fi.path()+"/";
        QTextStream textStream(&file);

        while( !textStream.atEnd() ) {
            QString line = textStream.readLine().trimmed();
            if( line.startsWith("background-image:url(") ) {
                QString relative = line.remove(0, 21); // remove background...
                relative.prepend(base);
                line = QString( "background-image:url(%1").arg(relative);
            }
            line.append('\n');
            styles.append(line);
        }

        // qDebug() << "+++++++++++++++++++++++++++++++++++++++++++++" << styles;
        return styles;
    }
    return QString();
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
    const QString s = mStyles;

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

