
#include <QDate>

#include "xmldocument.h"
#include "kraftdoc.h"
#include "documentman.h"

XmlDocument::XmlDocument()
{
}

void XmlDocument::setKraftDoc( KraftDoc *doc )
{
    QString re;

    if( ! doc ) return;

    Client c;
    c.setAddress(doc->address());
    c.setClientId(doc->addressUid());
    setClient(c);

    Meta m;

    m.setCurrency(QLatin1String("EUR"));
    m.setDocDesc(doc->whiteboard());
    m.setCountry(doc->country());
    m.setLanguage(doc->language());
    m.setOwner(QString::fromLocal8Bit( qgetenv("USER")));

    Tax::List taxes;
    Tax tr;
    tr.setType(QLatin1String("reduced"));
    tr.setValue(QString::number(DocumentMan::self()->reducedTax( doc->date()) ));
    taxes.append(tr);
    Tax tf;
    tr.setType(QLatin1String("full"));
    tf.setValue(QString::number(DocumentMan::self()->tax(doc->date())));
    taxes.append(tf);
    m.setTaxList(taxes);
    setMeta(m);

    Header h;
    h.setDocType(doc->docType());
    h.setIdent(doc->ident());
    h.setPreText(doc->preText());
    h.setProject(doc->projectLabel());
    h.setSalut(doc->salut());
    QString dateStr = doc->date().toString(Qt::ISODate);
    h.setDate(dateStr);
    setHeader(h);

    Footer f;
    f.setPostText(doc->postText());
    f.setGoodbye(doc->goodbye());
    setFooter(f);

    QString timeStr;
    timeStr = doc->lastModified().toString(Qt::ISODate);
    setLastModified(timeStr);
    setVersion(1);

}
