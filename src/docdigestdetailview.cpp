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

#include <khtmlview.h>
#include <kstandarddirs.h>

#include "docdigest.h"
#include "docdigestdetailview.h"
#include "defaultprovider.h"
#include "doctype.h"

#include "htmlview.h"
#include "texttemplate.h"

DocDigestHtmlView::DocDigestHtmlView( QWidget *parent )
    : HtmlView( parent )
{

}

bool DocDigestHtmlView::urlSelected( const QString &url, int, int,
                                     const QString &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &)
{
    kDebug() << "HtmlView::urlSelected(): " << url << endl;
    QRegExp rx("#show_last_print\\?id=(\\d+)");
    QRegExp rx2("#mark_sent\\?id=(\\d+)");
    QRegExp rxPayment("#set_payment\\?id=(\\d+)");

    bool ok = false;
    if ( rx.exactMatch( url ) ) {
        const QString idStr = rx.capturedTexts()[0];
        kDebug() << "Emitting showLastPrint";
        emit( showLastPrint( dbID( idStr.toInt( &ok ) ) ) );
    } else if( rx2.exactMatch( url )){
        const QString idStr = rx.capturedTexts()[0];
        kDebug() << "Emitting markSent";
        emit( markLastArchivedSent( dbID( idStr.toInt( &ok ) ) ) );
    } else if( rxPayment.exactMatch( url )) {
        const QString idStr = rxPayment.capturedTexts()[0];
        kDebug() << "Payment dialog for id" << idStr;
        emit( setPayment( dbID( idStr.toInt(&ok))));
    } else {
        kDebug() << "unknown action " << url << endl;
    }
    return ok;
}

// #########################################################################################################

DocDigestDetailView::DocDigestDetailView(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    setLayout( hbox );
    mHtmlCanvas = new DocDigestHtmlView( this );

    connect( mHtmlCanvas, SIGNAL(showLastPrint( const dbID& )),
             this, SIGNAL( showLastPrint( const dbID& ) ) );
    connect( mHtmlCanvas, SIGNAL(markLastArchivedSent( const dbID& )),
             this, SIGNAL( markLastArchivedSent(const dbID& ) ) );
    connect( mHtmlCanvas, SIGNAL(setPayment(dbID)),
             this, SIGNAL( setPayment(dbID)));

    QString fi = KStandardDirs::locate( "data", "kraft/reports/images/docdigestdetailview/kraft_customer.png" );

    QFileInfo info(fi);
    if( info.exists() ) {
        kDebug() << "Setting image base for docdigestDetailView: " << info.dir().absolutePath();
        mHtmlCanvas->setBaseUrl( info.dir().absolutePath() +"/" );
    } else {
        QByteArray home = qgetenv( "KRAFT_HOME" );
        if( !home.isEmpty() ) {
            QString burl = QString( "%1/reports/pics/").arg(QString::fromLocal8Bit( home ));
            kDebug() << "Setting base url from KRAFT_HOME: " << burl;
            mHtmlCanvas->setBaseUrl( burl );
        }
    }

    hbox->addWidget( mHtmlCanvas->view() );
}

#define DOCDIGEST_TAG

void DocDigestDetailView::slotShowDocDetails( DocDigest digest )
{
    kDebug() << "Showing details about this doc: " << digest.id();

    if( mTemplFile.isEmpty() ) {
        KStandardDirs stdDirs;
        const QString findFile = QLatin1String("kraft/reports/docdigest.trml");

        QString tmplFile = stdDirs.findResource( "data", findFile );

        if ( tmplFile.isEmpty() ) {
            QByteArray kraftHome = qgetenv("KRAFT_HOME");

            if( !kraftHome.isEmpty() ) {
                QString file = QString( "%1/reports/docdigest.trml").arg(QString::fromLocal8Bit(kraftHome));
                QFileInfo fi(file);
                if( fi.exists() && fi.isReadable() ) {
                    tmplFile = file;
                }
            }
            if( tmplFile.isEmpty() ) {
                kDebug() << "Could not find template to render document digest.";
                return;
            }
        }
        mTemplFile = tmplFile;
    }

    TextTemplate tmpl( mTemplFile ); // template file with name docdigest.trml
    if( !tmpl.open() ) {
        return;
    }
    tmpl.setValue( DOCDIGEST_TAG( "HEADLINE" ), digest.type() + " " + digest.ident() );

    tmpl.setValue( DOCDIGEST_TAG( "DATE" ), digest.date() );
    tmpl.setValue( DOCDIGEST_TAG( "DATE_LABEL" ), i18n("Date") );

    tmpl.setValue( DOCDIGEST_TAG( "WHITEBOARD"), digest.whiteboard() );
    tmpl.setValue( DOCDIGEST_TAG( "WHITEBOARD_LABEL"),            i18n("Whiteboard"));

    // some translations
    tmpl.setValue( DOCDIGEST_TAG("LAST_PRINTED_LABEL"),           i18n("Last printed:"));
    tmpl.setValue( DOCDIGEST_TAG("LAST_DOC_SHOW_TITLE_LABEL"),    i18n("opens last created PDF document."));
    tmpl.setValue( DOCDIGEST_TAG("MARKED_SENT_LABEL"),            i18n("This document was sent to the client on"));
    tmpl.setValue( DOCDIGEST_TAG("PAYMENT_RECEIVED_LABEL"),       i18n("was received as payment"));
    tmpl.setValue( DOCDIGEST_TAG("PAYMENT_AS_EXPECTED_LABEL"),    i18n("as expected."));
    tmpl.setValue( DOCDIGEST_TAG("PAYMENT_DIFFERENCE_LABEL"),     i18n(", that is a difference of"));
    tmpl.setValue( DOCDIGEST_TAG("PAYMENT_EXPECTATION_LABEL"),    i18n("expected as payment."));
    tmpl.setValue( DOCDIGEST_TAG("MARK_LAST_PRINTED_SENT_LABEL"), i18n("marks last printed doc as sent to customer"));
    tmpl.setValue( DOCDIGEST_TAG("NEVER_PRINTED_LABEL"),          i18n("This document was never printed"));

    if( !digest.projectLabel().isEmpty() ) {
        tmpl.createDictionary( "PROJECT_INFO" );
        tmpl.setValue( "PROJECT_INFO", DOCDIGEST_TAG( "PROJECT"), digest.projectLabel() );
        tmpl.setValue( "PROJECT_INFO", DOCDIGEST_TAG( "PROJECT_LABEL"), i18n("Project"));
    }

    tmpl.setValue( "URL", mHtmlCanvas->baseURL().prettyUrl());
    tmpl.setValue( DOCDIGEST_TAG( "CUSTOMER_LABEL" ), i18n("Customer"));

    KABC::Addressee addressee = digest.addressee();
    QString adr = digest.clientAddress();
    adr.replace('\n', "<br/>" );

    tmpl.setValue( DOCDIGEST_TAG("CUSTOMER_ADDRESS_FIELD"),adr );

    QString addressBookInfo;
    if( addressee.isEmpty() ) {
        if( digest.clientId().isEmpty() ) {
            addressBookInfo = i18n("The address is not listed in an address book.");
        } else {
            addressBookInfo = i18n("The client has the address book id %1 but can not found in our address books.").arg(digest.clientId());
        }
    } else {
        addressBookInfo  = i18n("The client can be found in our address books.");
        tmpl.createDictionary( "CLIENT_ADDRESS_SECTION");
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENTID" ), digest.clientId() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_ADDRESS" ), digest.clientAddress() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_NAME"), addressee.realName() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_ORGANISATION"), addressee.organization() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_URL"), addressee.url().prettyUrl() );
        tmpl.setValue( "CLIENT_ADDRESS_SECTION", DOCDIGEST_TAG( "CLIENT_EMAIL"), addressee.preferredEmail() );

        KABC::Address clientAddress;
        clientAddress = addressee.address( KABC::Address::Pref );
        QString addressType = i18n("preferred address");

        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KABC::Address::Home );
            addressType = i18n("home address");
        }
        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KABC::Address::Work );
            addressType = i18n("work address");
        }
        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KABC::Address::Postal );
            addressType = i18n("postal address");
        }
        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KABC::Address::Intl );
            addressType = i18n("international address");
        }
        if( clientAddress.isEmpty() ) {
            clientAddress = addressee.address( KABC::Address::Dom );
            addressType = i18n("domestic address");
        }

        if( clientAddress.isEmpty() ) {
            addressType = i18n("unknown");
            kDebug() << "WRN: Address is still empty!";
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

    // Information about archived documents.
    ArchDocDigestList archDocs = digest.archDocDigestList();
    DocType dt( digest.type() );

    if( archDocs.isEmpty() ) {
        kDebug() << "No archived docs for this document!";
        tmpl.createDictionary( DOCDIGEST_TAG( "NEVER_PRINTED" ));
        tmpl.setValue( "NEVER_PRINTED", DOCDIGEST_TAG("ARCHDOCS_TAG"), i18n("This document was never printed."));
    } else {
        ArchDocDigest archDocDigest = archDocs[0];
        ArchDoc archDoc(archDocDigest.archDocId());
        KLocale *docLocale = archDoc.locale();
        tmpl.createDictionary("PRINTED");
        tmpl.setValue( "PRINTED", DOCDIGEST_TAG("LAST_PRINT_DATE"), DefaultProvider::self()->locale()->formatDateTime(archDocDigest.printDate()));
        tmpl.setValue( "PRINTED", DOCDIGEST_TAG("LAST_PRINTED_ID"), archDocDigest.archDocId().toString() );
        tmpl.setValue( "PRINTED", DOCDIGEST_TAG("ARCHIVED_COUNT"), QString::number( archDocs.count()-1 ) );

        if( archDocDigest.hasDocState(ArchDoc::Sent) ) {
            tmpl.createDictionary("MARKED_SENT");
            QString dateStr = DefaultProvider::self()->locale()->formatDateTime(archDocDigest.sentOutDate(), KLocale::ShortDate );

            tmpl.setValue( "MARKED_SENT", DOCDIGEST_TAG("MARKED_SENT_DATE"), dateStr);
        } else {
            tmpl.createDictionary(DOCDIGEST_TAG("NOT_MARKED_SENT"));
            tmpl.setValue( "NOT_MARKED_SENT", DOCDIGEST_TAG("LAST_PRINTED_ID"), archDocDigest.archDocId().toString() );
        }

        // Payment information.
        if( dt.paymentExpected() ) {
            tmpl.createSubDictionary("MARKED_SENT", DOCDIGEST_TAG("PAYMENT"));
            if( archDocDigest.hasDocState(ArchDocAttributer::Payed) ) {
                tmpl.createSubDictionary("PAYMENT", DOCDIGEST_TAG("PAYED"));
                tmpl.setValue("PAYED", DOCDIGEST_TAG("PAYED_AMOUNT"), archDocDigest.payment().toHtmlString(docLocale ) );
                if( archDoc.bruttoSum() != archDocDigest.payment() ) {
                    tmpl.createSubDictionary("PAYMENT", DOCDIGEST_TAG("#PAYMENT_DIFFERENCE"));
                    Geld diff = archDocDigest.payment()-archDoc.bruttoSum();
                    tmpl.setValue("PAYMENT_DIFFERENCE", DOCDIGEST_TAG("DIFFERENCE"), diff.toHtmlString(docLocale) );
                } else {
                    tmpl.createSubDictionary("PAYMENT", DOCDIGEST_TAG("#SUM_AS_EXPECTED"));
                }
            } else {
                tmpl.createSubDictionary("PAYMENT", DOCDIGEST_TAG("PAYMENT_EXPECTED"));
                tmpl.setValue("PAYMENT_EXPECTED", DOCDIGEST_TAG("AMOUNT_TO_PAY"), archDoc.bruttoSum().toHtmlString(docLocale ) );
                tmpl.setValue("PAYMENT_EXPECTED", DOCDIGEST_TAG("LAST_PRINTED_ID"), archDocDigest.archDocId().toString() );
            }
        }
    }

    const QString details = tmpl.expand();
    mHtmlCanvas->displayContent( details );

    kDebug() << "BASE-URL of htmlview is " << mHtmlCanvas->baseURL();


}
