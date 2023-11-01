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

#include <QtGui>
#include <QDebug>
#include <QHBoxLayout>
#include <QSqlQuery>

#include <klocalizedstring.h>

#include "docdigest.h"
#include "docdigestdetailview.h"
#include "defaultprovider.h"
#include "htmlview.h"
#include "texttemplate.h"
#include "archdoc.h"
#include "format.h"
#include "grantleetemplate.h"

#include "xmldocindex.h"

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

    const QString idStr = q.queryItemValue(QLatin1String("id"));

    const QString path = url.path();
    bool ok;
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


  // --- The Actions Box
  QWidget *w = new QWidget;
  _docActionsWidget = new Ui::docActionsWidget;
  _docActionsWidget->setupUi(w);
  connect(_docActionsWidget->pbOpenPDF, &QPushButton::clicked,
          this, &DocDigestDetailView::openPDF);
  connect(_docActionsWidget->pbPrintDoc, &QPushButton::clicked,
          this, &DocDigestDetailView::printPDF);
  connect(_docActionsWidget->pbFinDoc, &QPushButton::clicked,
          this, &DocDigestDetailView::docStatusChange);
  connect(_docActionsWidget->pbXRechnung, &QPushButton::clicked,
          this, &DocDigestDetailView::exportXRechnung);

  hbox->addWidget(w);

  // --- The middle HTML based view
  hbox->setMargin(0);
  setLayout( hbox );
  mHtmlCanvas = new DocDigestHtmlView( this );
  mHtmlCanvas->setFrameStyle(0);
  mHtmlCanvas->setStylesheetFile("docdigestview.css");

  connect( mHtmlCanvas, SIGNAL(showLastPrint( const dbID& )),
           this, SIGNAL( showLastPrint( const dbID& ) ) );
  connect( mHtmlCanvas, SIGNAL(exportXRechnung( const dbID& )),
           this, SIGNAL( exportXRechnung( const dbID& ) ) );

  hbox->addWidget( mHtmlCanvas);

  const QString bgColor = mHtmlCanvas->palette().base().color().name();
  const QString style = QString("QLabel { "
                                "background-color: %1; "
                                "background-image: url(:/kraft/kraft_customer.png); background-repeat: repeat-none;"
                                "background-position: top left; "
                                "padding: 10px; "
                                "}").arg(bgColor);

  _leftDetails->setStyleSheet(style);

  // --- The right details Box
  const QString styleR = QString("QLabel { "
                                 "background-color: %1;"
                                 "background-image: url(:/kraft/postit.png); background-repeat: repeat-none;"
                                 "background-position: top center;"
                                 "padding: 0px; "
                                 "padding-left: 10px; "
                                 "}").arg(bgColor);


  _rightDetails = new QLabel(this);
  _rightDetails->setTextFormat(Qt::RichText);
  _rightDetails->setStyleSheet(styleR);
  _rightDetails->setMinimumWidth(detailMinWidth);
  _rightDetails->setWordWrap(true);
  _rightDetails->setTextInteractionFlags(Qt::TextSelectableByMouse);

  hbox->addWidget(_rightDetails);
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
            style += QLatin1String("padding-top: 95px; ");
        } else if( det == Month ) {
            image = "Calendar_page.png";
            bgPos = "center top";
            style += QLatin1String("padding-top: 75px; ");
        } else {
            // Document
            image = "kraft_customer.png";
            bgPos = "top left";
            style += QLatin1String( "padding-top: 50px; padding-left:15px;");
        }
    } else if(loc == Middle ) {
        if( det == Year ) {

        } else if( det == Month ) {

        } else {
            // Document
        }

    } else if(loc == Right ) {
        if( det == Year ) {

        } else if( det == Month ) {

        } else {
            // Document
            image = "postit.png";
            bgPos = "top center";
            style += QLatin1String("padding: 0px; padding-left: 30px; ");
        }

    } else {
        // undef.
    }

    if( !image.isEmpty() ) {
        style += QString("background-image: url(:/kraft/%1); background-repeat: repeat-none;"
                         "background-position: %2;").arg(image).arg(bgPos);
    }
    style += QLatin1String("}");
    return style;
}

#define DOCDIGEST_TAG

void DocDigestDetailView::documentListing( TextTemplate *tmpl, int year, int month )
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

    // read data in the given timeframe from database
    QSqlQuery q;
    const QString query = QString("SELECT archDocID, ident, MAX(printDate) FROM archdoc WHERE "
                                  "date BETWEEN date('%1') AND date('%2') "
                                  "GROUP BY ident").arg(minDate, maxDate);

    // qDebug() << "***" << query;
    QMap<QString, QPair<int, Geld> > docMatrix;
    q.prepare(query);
    q.exec();
    while( q.next() ) {
       dbID archDocId(q.value(0).toInt());

       const ArchDoc doc(archDocId);
       const QString docType = doc.docTypeStr();
       Geld g;
       int n = 0;
       if( docMatrix.contains(docType)) {
           g = docMatrix[docType].second;
           n = docMatrix[docType].first;
       }
       Geld g1 = doc.nettoSum();
       g += g1;
       docMatrix[docType].first = n+1;
       docMatrix[docType].second = g;
     }

    // now create the template

    tmpl->setValue("I18N_AMOUNT", i18n("Amount"));
    tmpl->setValue("I18N_TYPE",   i18n("Type"));
    tmpl->setValue("I18N_SUM",    i18n("Sum"));

    QStringList doctypes = docMatrix.keys();
    doctypes.sort();

    foreach( const QString dtype, doctypes ) {
        qDebug() << "creating doc list for "<<dtype;
        tmpl->createDictionary( "DOCUMENTS" );
        tmpl->setValue("DOCUMENTS", "DOCTYPE", dtype);
        const QString am = QString::number(docMatrix[dtype].first);
        tmpl->setValue("DOCUMENTS", "AMOUNT", am);
        const QString sm = docMatrix[dtype].second.toLocaleString();
        tmpl->setValue("DOCUMENTS", "SUM", sm);
    }
}

void DocDigestDetailView::slotShowMonthDetails( int year, int month )
{
    if( _monthTemplFileName.isEmpty() ) {
        _monthTemplFileName = DefaultProvider::self()->locateFile( "views/monthdigest.thtml" );
    }

    TextTemplate tmpl;
    tmpl.setTemplateFileName(_monthTemplFileName);
    if( !tmpl.isOk() ) {
        return;
    }
    const QString monthStr = DefaultProvider::self()->locale()->monthName(month);
    const QString yearStr = QString::number(year);
    tmpl.setValue( DOCDIGEST_TAG("HEADLINE"), i18n("Results in %1 %2", monthStr, yearStr) );
    tmpl.setValue( DOCDIGEST_TAG("YEAR_LABEL"), i18n("Year"));
    tmpl.setValue( DOCDIGEST_TAG("YEAR_NUMBER"), yearStr);
    tmpl.setValue( DOCDIGEST_TAG("MONTH_LABEL"), i18n("Month"));
    tmpl.setValue( DOCDIGEST_TAG("MONTH_NAME"), monthStr);

    // Document listing
    documentListing(&tmpl, year, month);

    // left and right information blocks
    _leftDetails->setStyleSheet(widgetStylesheet(Left, Month));
    _leftDetails->setText( "<h1>"+monthStr + "<br/>" + yearStr + "</h1>");
    _leftDetails->setAlignment(Qt::AlignHCenter);

    _rightDetails->setStyleSheet(widgetStylesheet(Right, Month));
    _rightDetails->clear();
    const QString details = tmpl.expand();
    mHtmlCanvas->displayContent(details);

}

void DocDigestDetailView::slotShowYearDetails( int year )
{
    if( _yearTemplFileName.isEmpty() ) {
        _yearTemplFileName = DefaultProvider::self()->locateFile( "views/yeardigest.thtml" );
    }

    TextTemplate tmpl;
    tmpl.setTemplateFileName(_yearTemplFileName);
    if( !tmpl.isOk() ) {
        return;
    }

    const QString yearStr = QString::number(year);
    tmpl.setValue( DOCDIGEST_TAG("YEAR_LABEL"), i18n("Year"));
    tmpl.setValue( DOCDIGEST_TAG("YEAR_NUMBER"), yearStr);
    tmpl.setValue( DOCDIGEST_TAG("HEADLINE"), i18n("Results in Year %1", yearStr) );

    documentListing(&tmpl, year, -1);

    const QString details = tmpl.expand();
    _leftDetails->setStyleSheet(widgetStylesheet(Left, Year));
    _leftDetails->setText("<h1>"+ yearStr +"</h1>");
    _leftDetails->setAlignment(Qt::AlignHCenter);

    _rightDetails->setStyleSheet(widgetStylesheet(Right, Year));
    _rightDetails->clear();

    mHtmlCanvas->displayContent( details );

}

void DocDigestDetailView::showAddress( const KContacts::Addressee& addressee, const QString& manAddress )
{
    Q_UNUSED(addressee)
    QString content = "<h3>" + i18n("Customer") +"</h3>";
    if( !manAddress.isEmpty() ) {
        content += "<pre>" + manAddress +"</pre>";
    } else {
        content += QLatin1String("<p>")+i18n("not set")+QLatin1String("</p>");
    }
    _leftDetails->setText( content );
#if 0
    // tmpl.setValue( "URL", mHtmlCanvas->baseURL().prettyUrl());
    tmpl.setValue( DOCDIGEST_TAG( "CUSTOMER_LABEL" ), i18n("Customer"));

    KContacts::Addressee addressee = digest.addressee();
    QString adr = digest.clientAddress();
    adr.replace('\n', "<br/>" );

    tmpl.setValue( DOCDIGEST_TAG("CUSTOMER_ADDRESS_FIELD"),adr );

    QString addressBookInfo;
    if( addressee.isEmpty() ) {
        if( digest.clientId().isEmpty() ) {
            addressBookInfo = i18n("The address is not listed in an address book.");
        } else {
            addressBookInfo = i18n("The client has the address book id %1 but can not found in our address books.", digest.clientId());
        }
    } else {
        addressBookInfo  = i18n("The client can be found in our address books.");
        tmpl.createDictionary( "CLIENT_ADDRESS_SECTION");
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENTID" ), digest.clientId() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_ADDRESS" ), digest.clientAddress() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_NAME"), addressee.realName() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_ORGANISATION"), addressee.organization() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_URL"), addressee.url().toString() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_EMAIL"), addressee.preferredEmail() );

        KContacts::Address clientAddress;
        clientAddress = addressee.address( KContacts::Address::Pref );
        QString addressType = i18n("preferred address");

        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KContacts::Address::Home );
            addressType = i18n("home address");
        }
        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KContacts::Address::Work );
            addressType = i18n("work address");
        }
        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KContacts::Address::Postal );
            addressType = i18n("postal address");
        }
        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KContacts::Address::Intl );
            addressType = i18n("international address");
        }
        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KContacts::Address::Dom );
            addressType = i18n("domestic address");
        }

        if( clientAddress.isEmpty() ) {
            addressType = i18n("unknown");
            // qDebug () << "WRN: Address is still empty!";
        }

        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_POSTBOX" ), clientAddress.postOfficeBox() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_EXTENDED" ), clientAddress.extended() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_STREET" ), clientAddress.street() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_LOCALITY" ), clientAddress.locality() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_REGION" ), clientAddress.region() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_POSTCODE" ), clientAddress.postalCode() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_COUNTRY" ),  clientAddress.country() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_REGION" ), clientAddress.region() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_LABEL" ), clientAddress.label() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_ADDRESS_TYPE" ), addressType );

    }
    tmpl.setValue( DOCDIGEST_TAG("CUSTOMER_ADDRESSBOOK_INFO"), addressBookInfo );
#endif
}

void DocDigestDetailView::slotEnablePDFActions(const DocDigest& digest)
{
    XmlDocIndex indx;
    const QFileInfo fi = indx.pdfPathByUuid(digest.uuid());
    bool pdfAvail = fi.exists();

    _docActionsWidget->pbOpenPDF->setEnabled(pdfAvail);
    _docActionsWidget->pbPrintDoc->setEnabled(pdfAvail);

}

void DocDigestDetailView::slotShowDocDetails( const DocDigest& digest )
{
    // qDebug () << "Showing details about this doc: " << digest.id();

    QObject obj;
    QObject labels;

    if( _docTemplFileName.isEmpty() ) {
        // QString templFileName = QString( "kraftdoc_%1_ro.gtmpl" ).arg( doc->docType() );
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

    labels.setProperty("date", i18n("Date"));
    labels.setProperty("exportXRechnungTitle", i18n("Export the invoice in XRechnung file format"));
    labels.setProperty("exportXRechnung", i18n("XRechnung"));
    labels.setProperty("whiteboard", i18n("Whiteboard"));
    labels.setProperty("project", i18n("Project"));
    labels.setProperty("state", i18n("State"));
    labels.setProperty("ident", i18n("Document Nr."));

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

    slotEnablePDFActions(digest);
}
