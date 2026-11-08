#include <QTest>

#include <QTemporaryDir>
#include <QFile>
#include <xmlindexdb.h>

#include "qtestcase.h"
#include "testconfig.h"
#include "documentsaverxml.h"
#include "grantleeallvarstemplate.h"
#include "kraftcontact.h"
#include "doctype.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "docposition.h"
#include "documenttemplate.h"
#include "myidentity.h"

#include <QDomDocument>


using namespace Qt::StringLiterals;

// The example contact that is used as own identity as well as customer contact.
// It only provides the example values in the generated variable list.
KContacts::Addressee example_contact()
{
    KContacts::Addressee contact;
    contact.setName("Goofy Enterprises");
    contact.setFormattedName("Goofy Enterprises");
    contact.setOrganization("Goofy Enterprises");
    KContacts::ResourceLocatorUrl url;
    url.setUrl(QUrl("https://www.goofy.com"));
    contact.setUrl(url);

    KContacts::Address addr(KContacts::Address::Work | KContacts::Address::Pref);
    addr.setStreet("Steinstr. 34");
    addr.setLocality("Spradsdorf");
    addr.setPostalCode("92192");
    addr.setCountry("Germany");
    contact.insertAddress(addr);

    KContacts::Email email("info@goofy.com");
    email.setPreferred(true);
    contact.addEmail(email);

    KContacts::PhoneNumber phone("0912 211259", KContacts::PhoneNumber::Work);
    contact.insertPhoneNumber(phone);
    contact.insertPhoneNumber(KContacts::PhoneNumber("0912 232322", KContacts::PhoneNumber::Fax));

    return contact;
}

class T_AllTemplVars: public QObject {
    Q_OBJECT
private Q_SLOTS:

    // copies the example xml document to a temporary path that simulates the
    // storage tree of kraft.
    void initTestCase()
    {
        Q_ASSERT(_dir.isValid());

        // dir.path() returns the unique directory path
        _baseDir = DefaultProvider::self()->createV2BaseDir(_dir.path());
        QVERIFY(!_baseDir.isEmpty());

        const QString docPath = QString("%1/%2/").arg(_baseDir).arg(_subdir);
        QDir d(docPath);
        d.mkpath(docPath);
        const QString filePath = d.filePath(_docUuid+".xml");
        qDebug() << "Example document path: " << docPath;

        QVERIFY(!_sourceDir.isEmpty());
        const QString src{_sourceDir + "/xml/kraftdoc.xml"};
        QVERIFY(QFile::copy(src, filePath));
        qDebug() << "Copied from" << src << "to filepath" << filePath;

        // The same document without any VAT, as a small business writes it.
        QFile srcFile(src);
        QVERIFY(srcFile.open(QIODevice::ReadOnly));
        QString noTax = QString::fromUtf8(srcFile.readAll());
        noTax.replace(_docUuid, _noTaxDocUuid).replace(_docIdent, u"20110128"_s);
        noTax.replace(u"<taxtype>Full</taxtype>"_s, u"<taxtype>None</taxtype>"_s);
        noTax.replace(u"<taxtype>Reduced</taxtype>"_s, u"<taxtype>None</taxtype>"_s);
        // an item text with two lines and a char that needs XML escaping
        noTax.replace(u"<text>first item</text>"_s, u"<text>first item &amp; more\nsecond line</text>"_s);
        // a free text buyer reference that needs XML escaping
        noTax.replace(u"<buyerReference>04011000-1234512345-06</buyerReference>"_s,
                      u"<buyerReference>Order A&amp;B</buyerReference>"_s);
        QFile noTaxFile(d.filePath(_noTaxDocUuid + ".xml"));
        QVERIFY(noTaxFile.open(QIODevice::WriteOnly));
        noTaxFile.write(noTax.toUtf8());
        noTaxFile.close();

        // generate the index
        XmlIndexDb indx;

        QFileInfo fi(docPath);
        QVERIFY(fi.exists());

        // The EPC QR code is only generated if the bank account is configured.
        // Set example values to not depend on the local configuration.
        KraftSettings::self()->setBankAccountName("Goofy Enterprises");
        KraftSettings::self()->setBankAccountIBAN("DE02120300000000202051");
        KraftSettings::self()->setBankAccountBIC("BYLADEM1001");

        // Same for the business data of the own identity, only in memory, not saved.
        KraftContact *me = MyIdentity::ownContact();
        me->setStringAttribute(KraftContact::VatId, u"DE123456789"_s);
        me->setStringAttribute(KraftContact::TaxNumber, u"143/123/45678"_s);
        me->setStringAttribute(KraftContact::RegistrationId, u"HRB 4324"_s);
        me->setStringAttribute(KraftContact::LegalForm, u"GbR"_s);
    }

    // The example document is of doc type Rechnung, which has to exist in the
    // simulated storage tree, otherwise the document can not be opened.
    void generateRechnungDoctype()
    {
        DocTypes dts;
        DocType dt;
        dt.setName("Rechnung");
        dt.setAppendPDFFile("appender.pdf");
        dt.setTemplateFile("mytemplate.gtmpl");
        dt.setWatermarkFile("mywatermark.pdf");
        dt.setXRechnungTemplate("myxrechnungstemplate.xml");
        dt.setMergeIdent(3);
        dt.setUuid("foo-bar-baz");
        const QStringList li{"Tag1", "Tag2", "Tag3"};
        dt.wipeAndSetTags(li);

        // the flags are stored as tags, so they have to be set after setTags()
        dt.setXRechnungEnabled(true);
        dt.setIsInvoice(true);

        const QStringList flist{"Auftragsbestätigung", "Rechnung", "Teilrechnung"};
        dt.setFollowers(flist);

        QCOMPARE(dts.save(dt, _baseDir), DocTypes::SaveResult::SaveOk);

        // a fresh instance has to find the doc type on disk
        DocTypes reloaded;
        QVERIFY(reloaded.allNames().contains("Rechnung"));
    }

    // Expands the asciidoc template with all known template variables and writes
    // the result to manual/templatevars.md, which is included by the manual.
    void generateDocInputFile()
    {
        const QString tmplFile{_sourceDir + "/reports/allvars.gtmpl"};
        QVERIFY2(QFileInfo::exists(tmplFile), qPrintable(tmplFile));

        QScopedPointer<GrantleeAllVarsTemplate> templateEngine(new GrantleeAllVarsTemplate(tmplFile));
        KraftContact contact;
        contact.setAddressee(example_contact());
        contact.setStringAttribute(KraftContact::BuyerReference, u"04011000-1234512345-06"_s);
        const QString expanded = templateEngine->expand(_docUuid, &contact, &contact);

        QVERIFY2(templateEngine->error().isEmpty(), qPrintable(templateEngine->error()));
        QVERIFY(!expanded.isEmpty());

        // the temp files, ie. the EPC QR code svg, are not needed here.
        const QStringList tmpFiles = templateEngine->tempFilesCreated();
        for (const QString& tmpFile : tmpFiles) {
            QFile::remove(tmpFile);
        }

        const QString f{_sourceDir + "/manual/templatevars.md"};
        QFile::remove(f);
        const QByteArray out{expanded.toUtf8()};

        QFile file(f);
        QVERIFY2(file.open(QIODevice::WriteOnly | QIODevice::Text), qPrintable(file.errorString()));
        QCOMPARE(file.write(out), out.size());
        QVERIFY2(file.flush(), qPrintable(file.errorString()));
        file.close();
        qDebug() << "Saved template variable list to" << f;
    }

    void xrechnungTemplate()
    {
        const QString out = expandXRechnung(_docUuid);
        QDomDocument dom;
        QVERIFY2(dom.setContent(out), qPrintable(out));

        // BT-55 comes from the country name of the customer address, Germany.
        QVERIFY(out.contains(u"<cbc:IdentificationCode>DE</cbc:IdentificationCode>"_s));
        // UBL demands Telephone before ElectronicMail in the customer contact.
        const QDomElement customerContact = dom.documentElement()
                                                .firstChildElement(u"cac:AccountingCustomerParty"_s)
                                                .firstChildElement(u"cac:Party"_s)
                                                .firstChildElement(u"cac:Contact"_s);
        QCOMPARE(customerContact.firstChildElement(u"cbc:Telephone"_s).nextSiblingElement().tagName(),
                 u"cbc:ElectronicMail"_s);
        QVERIFY(!out.contains(u"<cbc:ID>E</cbc:ID>"_s));
        // BT-9 and BT-10 come from the document
        QVERIFY(out.contains(u"<cbc:DueDate>2011-02-17</cbc:DueDate>"_s));
        QVERIFY(out.contains(u"<cbc:BuyerReference>04011000-1234512345-06</cbc:BuyerReference>"_s));
    }

    void xrechnungTemplateSmallBusiness()
    {
        KraftContact *own = MyIdentity::ownContact();
        const QString iban = own->stringAttribute(KraftContact::Iban);
        own->setStringAttribute(KraftContact::SmallBusiness, u"1"_s);
        own->setStringAttribute(KraftContact::Iban, u"DE02 1203 0000 0000 2020 51"_s);
        const QString out = expandXRechnung(_noTaxDocUuid, u"Goofy & Sons"_s);
        own->removeAttribute(KraftContact::SmallBusiness);
        own->setStringAttribute(KraftContact::Iban, iban);

        QDomDocument dom;
        QVERIFY2(dom.setContent(out), qPrintable(out));
        QVERIFY(out.contains(u"<cbc:TaxExemptionReason>Kleinunternehmer gemäß § 19 UStG</cbc:TaxExemptionReason>"_s));
        QVERIFY(out.contains(u"<cbc:ID>E</cbc:ID>"_s));
        QVERIFY(!out.contains(u"<cbc:ID>Z</cbc:ID>"_s));
        QVERIFY(!out.contains(u"<cbc:ID>S</cbc:ID>"_s));
        // escaped exactly once, the document and the contact values
        QVERIFY2(out.contains(u"<cbc:Name>first item &amp; more</cbc:Name>"_s), qPrintable(out));
        QVERIFY(out.contains(u"<cbc:Description>first item &amp; more\nsecond line</cbc:Description>"_s));
        QVERIFY(!out.contains(u"&lt;br/&gt;"_s));
        // BT-84 without the spaces of the printed IBAN
        QVERIFY(out.contains(u"<cbc:ID>DE02120300000000202051</cbc:ID>"_s));
        // every unit has a code, the lump sum unit gets C62
        QVERIFY(!out.contains(u"unitCode=\"\""_s));
        QVERIFY(out.contains(u"unitCode=\"C62\""_s));
        QVERIFY2(out.contains(u"<cbc:RegistrationName>Goofy &amp; Sons</cbc:RegistrationName>"_s), qPrintable(out));
        QVERIFY2(out.contains(u"<cbc:BuyerReference>Order A&amp;B</cbc:BuyerReference>"_s), qPrintable(out));
    }

private:
    QString expandXRechnung(const QString& uuid, const QString& customerOrg = QString())
    {
        GrantleeDocumentTemplate tmpl(_sourceDir + "/reports/xrechnung.xrtmpl");

        KraftContact *me = MyIdentity::ownContact();
        me->setAddressee(example_contact());
        KraftContact customer;
        KContacts::Addressee c = example_contact();
        if (!customerOrg.isEmpty())
            c.setOrganization(customerOrg);
        customer.setAddressee(c);

        const QString out = tmpl.expand(uuid, me, &customer);
        for (const QString& tmpFile : tmpl.tempFilesCreated()) {
            QFile::remove(tmpFile);
        }
        return out;
    }

    const QString _noTaxDocUuid{"f4deb784-6131-4e49-bd6d-92bd1355ea4e"};
    const QString _sourceDir {QDir(TESTS_PATH).filePath("..")};
    const QString _subdir {"/xmldoc/2011/01"};
    const QString _docIdent {"20110127"};
    const QString _docUuid{"f4deb784-6131-4e49-bd6d-92bd1355ea4d"};
    QTemporaryDir _dir;
    QString _baseDir;

};

QTEST_MAIN(T_AllTemplVars)

#include "t_alltemplvars.moc"
