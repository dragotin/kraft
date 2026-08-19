#include <QTest>

#include <QTemporaryDir>
#include <QFile>
#include <xmldocindex.h>

#include "qtestcase.h"
#include "testconfig.h"
#include "documentsaverxml.h"
#include "grantleeallvarstemplate.h"
#include "doctype.h"
#include "defaultprovider.h"
#include "kraftsettings.h"
#include "docposition.h"


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

        // generate the index
        XmlDocIndex indx;
        indx.setBasePath(_baseDir); // FIXME needs to go away

        QFileInfo fi(docPath);
        QVERIFY(fi.exists());

        // The EPC QR code is only generated if the bank account is configured.
        // Set example values to not depend on the local configuration.
        KraftSettings::self()->setBankAccountName("Goofy Enterprises");
        KraftSettings::self()->setBankAccountIBAN("DE02120300000000202051");
        KraftSettings::self()->setBankAccountBIC("BYLADEM1001");
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
        const KContacts::Addressee contact = example_contact();
        const QString expanded = templateEngine->expand(_docUuid, contact, contact);

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

private:
    const QString _sourceDir {QDir(TESTS_PATH).filePath("..")};
    const QString _subdir {"/xmldoc/2011/01"};
    const QString _docIdent {"20110127"};
    const QString _docUuid{"f4deb784-6131-4e49-bd6d-92bd1355ea4d"};
    QTemporaryDir _dir;
    QString _baseDir;

};

QTEST_MAIN(T_AllTemplVars)

#include "t_alltemplvars.moc"
