/***************************************************************************
             docdigestdetailview.cpp  - Details of a doc digest
                             -------------------
    begin                : februry 2011
    copyright            : (C) 2011 by Klaas Freitag
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
#include <array>

#include <QtGui>
#include <QDebug>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QToolButton>

#include <klocalizedstring.h>
#include <kcontacts/phonenumber.h>

#include "docdigest.h"
#include "docdigestdetailview.h"
#include "defaultprovider.h"
#include "htmlview.h"
#include "format.h"
#include "kraftsettings.h"
#include "grantleetemplate.h"

#include "xmldocindex.h"
#include "version.h"


DocDigestHtmlView::DocDigestHtmlView( QWidget *parent )
  : HtmlView( parent )
{
    connect(this, SIGNAL(openUrl(QUrl)), this, SLOT(slotLinkClicked(QUrl)));
}

void DocDigestHtmlView::slotLinkClicked(const QUrl& url)
{
    // This method is not longer required as there are no
    // links any more in the description. But its left here
    // as an example

    const QUrlQuery q(url);
    // Url is like "http://localhost/show_last_print?id=5"

    const QString idStr = q.queryItemValue(QStringLiteral("id"));

    const QString path = url.path();

    if( path.endsWith("show_last_print")) {
        // emit( .. );
    } else if (path.endsWith("export_xrechnung")) {
        // emit( .. );
    }
}

// #########################################################################################################

DocDigestDetailView::DocDigestDetailView(QWidget *parent) :
    QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel+QFrame::Raised);
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setSpacing(0);

  const int detailMinWidth = 260;
  setFixedHeight(200);
  // --- The left details box
  _leftDetails = new QLabel(this);
  hbox->addWidget(_leftDetails);
  _leftDetails->setTextFormat(Qt::RichText);
  _leftDetails->setMinimumWidth(detailMinWidth);
  _leftDetails->setFrameStyle(0);
  _leftDetails->setTextInteractionFlags(Qt::TextSelectableByMouse);

  _leftDetails->setWordWrap(true);

  // --- The middle HTML based view
  hbox->setContentsMargins(0,0,0,0);
  setLayout( hbox );
  mHtmlCanvas = new DocDigestHtmlView( this );
  mHtmlCanvas->setFrameStyle(0);
  mHtmlCanvas->setStylesheetFile("docdigestview.css");

  hbox->addWidget(mHtmlCanvas);

  const QString bgColor = mHtmlCanvas->palette().base().color().name();
  const QString style = widgetStylesheet(Location::Left, Detail::Start);

  _leftDetails->setStyleSheet(style);

  // --- The right details Box
  const QString styleR = widgetStylesheet(Location::Right, Detail::Start);

  _rightDetails = new QLabel(this);
  _rightDetails->setTextFormat(Qt::RichText);
  _rightDetails->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  _rightDetails->setStyleSheet(styleR);
  _rightDetails->setMinimumWidth(detailMinWidth);
  _rightDetails->setWordWrap(true);
  _rightDetails->setTextInteractionFlags(Qt::TextSelectableByMouse);

  hbox->addWidget(_rightDetails);

  slotShowStart();

  // --- The Actions Box
  QWidget *w = new QWidget;
  _docActionsWidget = new Ui::docActionsWidget;
  _docActionsWidget->setupUi(w);

  hbox->addWidget(w);
}

void DocDigestDetailView::initViewActions(const std::array<QAction*, 4> actions)
{
    qDebug() << "Sizeof actions" << actions.size();
    Q_ASSERT(_docActionsWidget != nullptr);

    _docActionsWidget->_tbEdit->setDefaultAction(actions[0]);
    _docActionsWidget->_tbFinalize->setDefaultAction(actions[1]);
    _docActionsWidget->_tbOpenPDF->setDefaultAction(actions[2]);
    _docActionsWidget->_tbPrintPDF->setDefaultAction(actions[3]);
}

void DocDigestDetailView::slotClearView()
{
    const QString details;
    mHtmlCanvas->displayContent( details );
}

QString DocDigestDetailView::widgetStylesheet( Location loc, Detail det )
{
    const QString bgColor = mHtmlCanvas->palette().base().color().name();
    QString style = QString("QLabel { background-color: %1; ").arg(bgColor);
    QString image;
    QString bgPos;

    if( loc == Left ) {
        if( det == Year ) {
            image = "Calendar_page.png";
            bgPos = "center top";
            style += QStringLiteral("padding-top: 95px; ");
        } else if( det == Month ) {
            image = "Calendar_page.png";
            bgPos = "center top";
            style += QStringLiteral("padding-top: 75px; ");
        } else if (det == Document){
            // Document
            image = QStringLiteral("users.png");
            bgPos = "top left";
            style += QStringLiteral( "padding-top: 65px; padding-left:15px; padding-right:8px;");
        } else if (det == Start) {
            image = QStringLiteral("kraft-simple.png");
            bgPos = "center";
        } else {

        }
    } else if(loc == Middle ) {
        if( det == Year ) {

        } else if( det == Month ) {

        } else if( det == Start) {

        } else {
            // Document
        }

    } else if(loc == Right ) {
        if( det == Year ) {

        } else if( det == Month ) {

        } else {
            // Document
            image = QStringLiteral("postit.png");
            bgPos = QStringLiteral("top center");
            style += QStringLiteral("padding: 0px; padding-top: 30px; padding-left: 5px; padding-right: 12px; ");
        }

    } else {
        // undef.
    }

    if( !image.isEmpty() ) {
        style += QString("background-image: url(:/kraft/%1); background-repeat: repeat-none;"
                         "background-position: %2;").arg(image).arg(bgPos);
    }
    style += QStringLiteral("}");
    return style;
}

#define DOCDIGEST_TAG

QList<QObject*> DocDigestDetailView::documentListing(int year, int month )
{

    QString minDate;
    QString maxDate;
    if( month > -1 ) {
        QDate theDate(year, month, 1);
        // not a year
        minDate = theDate.toString("yyyy-MM-dd");
        int lastDay = theDate.daysInMonth();
        theDate.setDate(year, month, lastDay);
        maxDate = theDate.toString("yyyy-MM-dd");
    } else {
        // is is a year
        minDate = QString::number(year)+"-01-01";
        maxDate = QString::number(year)+"-12-31";
    }

    // FIXME: overview of the sum of finalized docs
    // Generate a list of documents between min- and max date, grouped by doc type
    // The following template vars need to be set with the overall sums

    // now create the template

    QStringList doctypes;
    // QStringList doctypes = docMatrix.keys();
    // doctypes.sort();

    QList<QObject*> objList;
    for( const QString& dtype: doctypes ) {
        QObject *obj = new QObject;
        qDebug() << "creating doc list for "<<dtype;
        obj->setProperty("doctype", dtype);
        const QString am = QString::number(0);
        obj->setProperty("amount", am);
        const QString sm = Format::localeDoubleToString(0.0);
        obj->setProperty("sum", sm);

        objList.append(obj);
    }
    return objList;
}

void DocDigestDetailView::slotShowStart()
{
    const QString templateFile = DefaultProvider::self()->locateFile( "views/welcome.gtmpl" );

    GrantleeFileTemplate tmpl(templateFile);

    QString name = qgetenv("USER");
    if (name.isEmpty())
        name = qgetenv("USERNAME");

    QObject obj;
    obj.setProperty("version", Kraft::Version::number());
    obj.setProperty("codename", Kraft::Version::codeName());
    obj.setProperty("username", name);

    QObject labels;
    labels.setProperty("header", i18n("Welcome to Kraft"));
    labels.setProperty("version", i18n("Version"));
    labels.setProperty("text1", i18n("Kraft helps you to handle documents like quotes and invoices in your small business."));

    tmpl.addToObjMapping("kraft", &obj);
    tmpl.addToObjMapping("label", &labels);
    bool ok;
    const QString welcome = tmpl.render(ok);
    mHtmlCanvas->displayContent(welcome);
}

void DocDigestDetailView::slotShowMonthDetails( int year, int month )
{
    if( _monthTemplFileName.isEmpty() ) {
        _monthTemplFileName = DefaultProvider::self()->locateFile( "views/monthdigest.gtmpl" );
    }

    GrantleeFileTemplate tmpl(_monthTemplFileName);
    if( !tmpl.isOk() ) {
        return;
    }

    QObject obj;
    const QString monthStr = DefaultProvider::self()->locale()->monthName(month);
    const QString yearStr = QString::number(year);
    obj.setProperty("headline", i18n("Results in %1 %2", monthStr, yearStr));
    obj.setProperty("year_number", yearStr);
    obj.setProperty("month_name", monthStr);

    QObject labels;
    labels.setProperty("year_label", i18n("Year"));
    labels.setProperty("month_label", i18n("Month"));
    labels.setProperty("amount", i18n("Amount"));
    labels.setProperty("type", i18n("Type"));
    labels.setProperty("sum", i18n("Sum"));

    // Document listing
    QList<QObject*> docList = documentListing(year, month);
    obj.setProperty("docList",QVariant::fromValue(docList));

    // left and right information blocks
    _leftDetails->setStyleSheet(widgetStylesheet(Left, Month));
    _leftDetails->setText( "<h1>"+monthStr + "<br/>" + yearStr + "</h1>");
    _leftDetails->setAlignment(Qt::AlignHCenter);

    _rightDetails->setStyleSheet(widgetStylesheet(Right, Month));
    _rightDetails->clear();

    tmpl.addToObjMapping("kraft", &obj);
    tmpl.addToObjMapping("label", &labels);
    bool ok;
    const QString details = tmpl.render(ok);
    mHtmlCanvas->displayContent(details);

}

void DocDigestDetailView::slotShowYearDetails( int year )
{
    if( _yearTemplFileName.isEmpty() ) {
        _yearTemplFileName = DefaultProvider::self()->locateFile( "views/yeardigest.gtmpl" );
    }
    GrantleeFileTemplate tmpl(_yearTemplFileName);
    if( !tmpl.isOk() ) {
        return;
    }

    QObject labels;
    labels.setProperty("year_label", i18n("Year"));

    QObject obj;
    const QString yearStr = QString::number(year);
    obj.setProperty("year_number", yearStr);
    obj.setProperty("headline", i18n("Results in Year %1", yearStr));

    QList<QObject*> docList = documentListing(year, -1);
    obj.setProperty("docList",QVariant::fromValue(docList));

    _leftDetails->setStyleSheet(widgetStylesheet(Left, Year));
    _leftDetails->setText("<h1>"+ yearStr +"</h1>");
    _leftDetails->setAlignment(Qt::AlignHCenter);

    _rightDetails->setStyleSheet(widgetStylesheet(Right, Year));
    _rightDetails->clear();

    tmpl.addToObjMapping("kraft", &obj);
    tmpl.addToObjMapping("label", &labels);
    bool ok;
    const QString details = tmpl.render(ok);

    mHtmlCanvas->displayContent( details );

}

void DocDigestDetailView::showAddress( const KContacts::Addressee& addressee, const QString& manAddress )
{
    Q_UNUSED(addressee)
    QString content;

    if( !manAddress.isEmpty() ) {
        content += QStringLiteral("<pre>") + manAddress + QStringLiteral("</pre>");
    } else {
        content += QStringLiteral("<p>")+i18n("not set")+QStringLiteral("</p>");
    }

    QString addressContent;
    if (addressee.isEmpty()) {
        addressContent += i18n("manual address");
    } else {
        const QString email = addressee.preferredEmail();
        if (!email.isEmpty()) {
            addressContent += i18n("Email:%1").arg(email.toHtmlEscaped());
        }
        const auto phone = addressee.phoneNumber(KContacts::PhoneNumber::Home);
        if (!phone.isEmpty()) {
            addressContent += i18n("Number (Home):%1").arg(phone.number());
        }
        const auto phoneW = addressee.phoneNumber(KContacts::PhoneNumber::Work);
        if (!phoneW.isEmpty()) {
            addressContent += i18n("Number (Work):%1").arg(phoneW.number());
        }
    }

    if (!addressContent.isEmpty()) {
        content += QStringLiteral("<hr/>");
        content += QStringLiteral("<span style=\" font-size:8pt;\">");
        content.append(addressContent);
        content += QStringLiteral("</span>");
    }

    _leftDetails->setText( content );
}

void DocDigestDetailView::setErrorStrings(const QString& header, const QString& details)
{
    qDebug() << "Set error strings" << header;
    slotShowDocDetails(_currentDigest, header, details);
}

void DocDigestDetailView::slotShowDocDetails(const DocDigest& digest, const QString& errHeader, const QString& errDetails)
{
    // qDebug () << "Showing details about this doc: " << digest.id();
    _currentDigest = digest;
    qDebug() << "Show doc details" << errHeader;

    QObject obj;
    QObject labels;

    showAddress(digest.addressee(), digest.clientAddress());

    if( _docTemplFileName.isEmpty() ) {
        _docTemplFileName = DefaultProvider::self()->locateFile( "views/docdigest.gtmpl" );
    }

    GrantleeFileTemplate gtmpl(_docTemplFileName);

    obj.setProperty("headline", digest.type());
    obj.setProperty("date", digest.date());
    obj.setProperty("isInvoice", true /* FIXME */);
    obj.setProperty("whiteboard", digest.whiteboard());
    if (!digest.projectLabel().isEmpty())
        obj.setProperty("project", digest.projectLabel());
    obj.setProperty("state", digest.stateStr());
    obj.setProperty("ident", digest.ident());
    const QString lmd = Format::toDateTimeString(digest.lastModified(), KraftSettings::self()-> dateFormat());
    obj.setProperty("modifiedDateDoc", lmd);

    obj.setProperty("creationError", ! errHeader.isEmpty());
    obj.setProperty("errorHeader",     errHeader);
    obj.setProperty("errorDetails",    errDetails);

    // PDF file info
    XmlDocIndex indx;
    const QFileInfo fi = indx.pdfPathByUuid(digest.uuid());
    bool pdfAvail = fi.exists();
    obj.setProperty("pdfAvailable", pdfAvail);
    if (pdfAvail) {
        const QString lmd = Format::toDateTimeString(fi.lastModified(), KraftSettings::self()-> dateFormat());
        obj.setProperty("modifiedDatePdf", lmd);
    }

    labels.setProperty("date", i18n("Date"));
    labels.setProperty("exportXRechnungTitle", i18n("Export the invoice in XRechnung file format"));
    labels.setProperty("exportXRechnung", i18n("XRechnung"));
    labels.setProperty("whiteboard", i18n("Whiteboard"));
    labels.setProperty("project", i18n("Project"));
    labels.setProperty("state", i18n("State"));
    labels.setProperty("ident", i18n("Document Nr."));
    labels.setProperty("doclastmodified", i18n("Document last modified"));
    labels.setProperty("pdflastmodified", i18n("PDF generated"));
    labels.setProperty("pdfnotavailable", i18n("PDF not yet generated"));

    gtmpl.addToObjMapping("doc", &obj);
    gtmpl.addToObjMapping("label", &labels);

    bool ok;
    const QString details = gtmpl.render(ok);
    mHtmlCanvas->displayContent( details );

    _rightDetails->setText(digest.whiteboard());
    _leftDetails->setStyleSheet(widgetStylesheet(Left, Document));
    _leftDetails->setAlignment(Qt::AlignLeft);

    _rightDetails->setStyleSheet(widgetStylesheet(Right, Document));
    // qDebug () << "BASE-URL of htmlview is " << mHtmlCanvas->baseURL();
}
