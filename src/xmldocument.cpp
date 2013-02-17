
#include <QDate>

#include "xmldocument.h"
#include "kraftdoc.h"
#include "documentman.h"
#include "docposition.h"
#include "einheit.h"
#include "geld.h"

XmlDocument::XmlDocument()
{
}

void XmlDocument::getKraftDoc( KraftDoc *doc )
{
    if( ! doc ) {
        return;
    }

    doc->setLastModified( QDate::fromString(lastModified()) );

    doc->setCountryLanguage( meta().country(), meta().language() );

    doc->setAddress( client().address() );
    doc->setAddressUid( client().clientId() );

    doc->setDate( QDate::fromString( header().date()) );
    // doc->setDocID(  );
    doc->setDocType( header().docType() );
    doc->setIdent( header().ident() );
    doc->setPreText( header().preText() );
    doc->setProjectLabel( header().project() );
    doc->setSalut( header().salut() );
    doc->setWhiteboard( header().whiteboard());
    QDate d = QDate::fromString( header().date(), Qt::ISODate );
    doc->setDate( d );
    doc->setProjectLabel( header().project() );

    doc->setGoodbye( footer().goodbye() );
    doc->setPostText( footer().postText() );

    // parse items.
    Items items = this->items();
    foreach( Item item, items.itemList() ) {
        DocPosition *dp = doc->createPosition(DocPositionBase::Position);
        bool ok;
        double amount = item.amount().toDouble(&ok);
        dp->setAmount( amount );
        dp->setPositionNumber( item.number() );
        dp->setText( item.text() );

        dp->setTaxType( item.taxType().toInt(&ok) );
        Einheit unit( item.unit() );
        dp->setUnit( unit );


        double money = item.unitprice().toDouble( &ok );
        Geld g(money);
        dp->setUnitPrice( g );

        ItemAttribute::List attribs = item.itemAttributeList();
        foreach( ItemAttribute attrib, attribs ) {
            if( attrib.name() == QLatin1String("tag") ) {
                dp->setTag( attrib.value() );
            } else {
                Attribute attribute( attrib.name() );
                attribute.setValue( attrib.value() );
                dp->setAttribute( attribute );
            }
        }
    }

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
    m.setCountry(doc->country());
    m.setLanguage(doc->language());
    m.setOwner(QString::fromLocal8Bit( qgetenv("USER")));

    Tax::List taxes;
    Tax tr;
    tr.setType(QLatin1String("reduced"));
    tr.setValue(QString::number(DocumentMan::self()->reducedTax( doc->date()) ));
    taxes.append(tr);
    Tax tf;
    tf.setType(QLatin1String("full"));
    tf.setValue(QString::number(DocumentMan::self()->fullTax(doc->date())));
    taxes.append(tf);
    m.setTaxList(taxes);
    setMeta(m);

    Header h;
    h.setDocType(doc->docType());
    h.setIdent(doc->ident());
    h.setPreText(doc->preText());
    h.setProject(doc->projectLabel());
    h.setSalut(doc->salut());
    h.setWhiteboard(doc->whiteboard());
    QString dateStr = doc->date().toString(Qt::ISODate);
    h.setDate(dateStr);
    setHeader(h);

    Item::List items;
    foreach( DocPositionBase* dpb, doc->positions() ) {
        Item item;
        DocPosition *dp = 0;
        switch( dpb->type() ) {
        case DocPositionBase::Position:
            dp = static_cast<DocPosition*>(dpb);
            item.setAmount(QString::number(dp->amount()));
            item.setNumber( dp->positionNumber());
            item.setUnit( QString::number(dp->unit().id()) );
            item.setUnitprice( QString::number( dp->unitPrice().toLong()) );
            item.setItemprice( QString::number( dp->overallPrice().toLong()) );
            break;
        case DocPositionBase::ExtraDiscount:
            break;
        case DocPositionBase::Header:
            break;
        default:
            break;
        }
        item.setText(dpb->text());

        QString tax;
        switch( dpb->taxType()) {
        case DocPositionBase::TaxNone:
            tax = QLatin1String("none");
            break;
        case DocPositionBase::TaxReduced:
            tax = QLatin1String("reduced");
            break;
        case DocPositionBase::TaxFull:
            tax = QLatin1String("full");
            break;
        default:
            tax = QLatin1String("error");
        }
        item.setTaxType( tax );

        AttributeMap attribs = dpb->attributes();

        foreach(Attribute attrib, attribs ) {
            ItemAttribute attr;
            QString name = attrib.name();
            QString val = attrib.value().toString();
            attr.setName(name);
            attr.setValue(val);
            item.addItemAttribute(attr);
        }

        items.append(item);
    }
    Items docItems;
    docItems.setItemList( items );
    setItems(docItems);

    // The sums.
    Sums  sums;
    Taxsum::List taxsums;
    Taxsum full;
    full.setType(QLatin1String("full"));
    Geld fullSum = doc->positions().fullTaxSum( DocumentMan::self()->fullTax( doc->date()));
    full.setValue(QString::number(fullSum.toLong()));
    taxsums.append(full);

    Taxsum reduced;
    reduced.setType(QLatin1String("reduced"));
    Geld redSum = doc->positions().reducedTaxSum( DocumentMan::self()->reducedTax( doc->date()));
    reduced.setValue(QString::number(redSum.toLong()));
    taxsums.append(reduced);
    sums.setTaxsumList(taxsums);

    sums.setBrutto( QString::number(doc->bruttoSum().toLong()) );
    sums.setNetto( QString::number(doc->nettoSum().toLong()) );
    setSums(sums);

    // The footer.
    Footer f;
    f.setPostText(doc->postText());
    f.setGoodbye(doc->goodbye());
    setFooter(f);

    QString timeStr;
    timeStr = doc->lastModified().toString(Qt::ISODate);
    setLastModified(timeStr);
    setVersion(1);

}
