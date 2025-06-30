/***************************************************************************
                          kraftview.cpp  -
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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

// include files for Qt
#include <qlayout.h>
#include <qlabel.h>
#include <qsizepolicy.h>
#include <qsignalmapper.h>
#include <qtabwidget.h>
#include <qcolor.h>
#include <qsplitter.h>
#include <qtooltip.h>
#include <qfont.h>
#include <qtimer.h>

#include <QDebug>
#include <QDialog>

// application specific includes
#include "kraftview_ro.h"
#include "kraftdoc.h"
#include "ui_docheader.h"
#include "positionviewwidget.h"
#include "ui_docfooter.h"
#include "docposition.h"
#include "defaultprovider.h"
#include "format.h"
#include "htmlview.h"
#include "documenttemplate.h"
#include "myidentity.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>


// #########################################################

KraftViewRO::KraftViewRO(QWidget *parent, const char *name) :
    KraftViewBase( parent )
{
  setObjectName( name );
  setModal( false );
  setWindowTitle( i18n("Document" ) );
  QWidget *mainWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(mainWidget);

  m_type = ReadOnly;

  mHtmlView = new HtmlView( this );
  mainLayout->addWidget(mHtmlView);
  mHtmlView->setStylesheetFile( "docoverview_ro.css" );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  mainLayout->addWidget(buttonBox);
}

KraftViewRO::~KraftViewRO()
{

}

#define QL1(X) QStringLiteral(X)

QString KraftViewRO::htmlify( const QString& str ) const
{
  const QStringList li = str.toHtmlEscaped().split( "\n" );
  return QL1("<p>") + li.join( "</p><p>" ) + QL1("</p>");
}

void KraftViewRO::setup(DocGuardedPtr doc)
{
    KraftViewBase::setup( doc );

    // use Grantlee.
    const QString tmplFile = DefaultProvider::self()->locateFile( "views/kraftdoc_ro.gtmpl" );

    GrantleeDocumentTemplate tmpl(tmplFile);

    // expand the template...
    KContacts::Addressee customerContact;
    // FIXME: Fill contacts with values
    MyIdentity identity;
    const QString uuid = doc->uuid();
    const QString html = tmpl.expand(uuid, identity.contact(), customerContact);
    const QStringList cleanupFiles = tmpl.tempFilesCreated();

    setWindowTitle(doc->docIdentifier());
    mHtmlView->setTitle( doc->docIdentifier() );
    mHtmlView->displayContent(html);
}

void KraftViewRO::slotLinkClicked(const QString& link)
{
    Q_UNUSED(link)
    // nothing we do here yet
}


void KraftViewRO::done( int r )
{
  // qDebug () << "View closed with ret value " << r;

  KraftDoc *doc = getDocument();

  if( !doc ) {
    // qDebug () << "ERR: No document available in view, return!";
    return;
  }

  Q_EMIT viewClosed( true, m_doc, false);

  KraftViewBase::done(r);
}

