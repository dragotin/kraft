#include <QTest>
#include <QDomDocument>
#include <QFile>
#include <QObject>
#include <QTemporaryDir>

#include "defaultprovider.h"
#include "documentman.h"
#include "kraftdoc.h"
#include "xmlindexdb.h"

class T_KraftDocState : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // KraftDoc::slotDeleteDoc() saves the document. Redirect the storage tree to
    // a temporary directory, otherwise the documents end up in the real Kraft
    // directory of the user running the test.
    void initTestCase()
    {
        QVERIFY(_dir.isValid());
        QVERIFY(!DefaultProvider::self()->createV2BaseDir(_dir.path()).isEmpty());
    }

    void defaultStateIsNew()
    {
        KraftDoc doc;
        QCOMPARE(doc.state().state(), KraftDocState::State::New);
        QVERIFY(doc.state().isNew());
        QVERIFY(doc.state().is(KraftDocState::State::New));
    }

    void stateStringRoundTrip_data()
    {
        QTest::addColumn<KraftDocState::State>("state");
        QTest::addColumn<QString>("expected");

        QTest::newRow("Undefined") << KraftDocState::State::Undefined << KraftDocState::StateUndefinedStr;
        QTest::newRow("New")       << KraftDocState::State::New       << KraftDocState::StateNewStr;
        QTest::newRow("Draft")     << KraftDocState::State::Draft     << KraftDocState::StateDraftStr;
        QTest::newRow("Final")     << KraftDocState::State::Final     << KraftDocState::StateFinalStr;
        QTest::newRow("Retracted") << KraftDocState::State::Retracted << KraftDocState::StateRetractedStr;
        QTest::newRow("Invalid")   << KraftDocState::State::Invalid   << KraftDocState::StateInvalidStr;
        QTest::newRow("Converted") << KraftDocState::State::Converted << KraftDocState::StateConvertedStr;
        QTest::newRow("Deleted")   << KraftDocState::State::Deleted   << KraftDocState::StateDeletedStr;
    }

    void stateStringRoundTrip()
    {
        QFETCH(KraftDocState::State, state);
        QFETCH(QString, expected);

        KraftDocState s;
        s.setState(state);
        QCOMPARE(s.stateString(), expected);

        KraftDocState s2;
        s2.setStateFromString(expected);
        QCOMPARE(s2.state(), state);
    }

    void setStateFromEmptyStringIsUndefined()
    {
        KraftDocState s;
        s.setState(KraftDocState::State::Final);
        s.setStateFromString(QString());
        QCOMPARE(s.state(), KraftDocState::State::Undefined);
    }

    void setStateFromUnknownStringIsInvalid()
    {
        KraftDocState s;
        s.setStateFromString(QStringLiteral("not-a-real-state"));
        QCOMPARE(s.state(), KraftDocState::State::Invalid);
    }

    void validFollowStates()
    {
        // From New: may transition to Deleted or Draft.
        const auto fromNew = KraftDocState::validFollowStates(KraftDocState::State::New);
        QVERIFY(fromNew.contains(KraftDocState::State::Deleted));
        QVERIFY(fromNew.contains(KraftDocState::State::Draft));
        QVERIFY(!fromNew.contains(KraftDocState::State::Final));

        // From Draft: may transition to Deleted or Final.
        const auto fromDraft = KraftDocState::validFollowStates(KraftDocState::State::Draft);
        QVERIFY(fromDraft.contains(KraftDocState::State::Deleted));
        QVERIFY(fromDraft.contains(KraftDocState::State::Final));

        // From Final: Retracted or Invalid. In particular, Deleted is NOT allowed.
        const auto fromFinal = KraftDocState::validFollowStates(KraftDocState::State::Final);
        QVERIFY(fromFinal.contains(KraftDocState::State::Retracted));
        QVERIFY(fromFinal.contains(KraftDocState::State::Invalid));
        QVERIFY(!fromFinal.contains(KraftDocState::State::Draft));
        QVERIFY(!fromFinal.contains(KraftDocState::State::Deleted));

        // From Invalid: may transition to Deleted.
        const auto fromInvalid = KraftDocState::validFollowStates(KraftDocState::State::Invalid);
        QVERIFY(fromInvalid.contains(KraftDocState::State::Deleted));

        // From Deleted: may be restored to Draft.
        const auto fromDeleted = KraftDocState::validFollowStates(KraftDocState::State::Deleted);
        QVERIFY(fromDeleted.contains(KraftDocState::State::Draft));
        QVERIFY(!fromDeleted.contains(KraftDocState::State::Deleted));

        // From Converted: may be deleted
        const auto fromConverted = KraftDocState::validFollowStates(KraftDocState::State::Converted);
        QVERIFY(fromConverted.contains(KraftDocState::State::Deleted));

        // Terminal states: no follow states.
        QVERIFY(KraftDocState::validFollowStates(KraftDocState::State::Retracted).isEmpty());
    }

    void canBeFinalized()
    {
        KraftDocState s;

        s.setState(KraftDocState::State::Draft);
        QVERIFY(s.canBeFinalized());

        s.setState(KraftDocState::State::New);
        QVERIFY(!s.canBeFinalized());

        s.setState(KraftDocState::State::Final);
        QVERIFY(!s.canBeFinalized());

        s.setState(KraftDocState::State::Deleted);
        QVERIFY(!s.canBeFinalized());
    }

    void forcesReadOnly()
    {
        KraftDocState s;

        s.setState(KraftDocState::State::Final);
        QVERIFY(s.forcesReadOnly());

        s.setState(KraftDocState::State::Invalid);
        QVERIFY(s.forcesReadOnly());

        s.setState(KraftDocState::State::Retracted);
        QVERIFY(s.forcesReadOnly());

        s.setState(KraftDocState::State::Draft);
        QVERIFY(!s.forcesReadOnly());

        s.setState(KraftDocState::State::Deleted);
        QVERIFY(!s.forcesReadOnly());
    }

    void deleteDocFromNew()
    {
        KraftDoc doc;
        QCOMPARE(doc.state().state(), KraftDocState::State::New);

        doc.slotDeleteDoc();
        QCOMPARE(doc.state().state(), KraftDocState::State::Deleted);
        QCOMPARE(doc.state().stateString(), KraftDocState::StateDeletedStr);
    }

    void deleteDocFromDraft()
    {
        KraftDoc doc;
        doc.state().setState(KraftDocState::State::Draft);

        doc.slotDeleteDoc();
        QCOMPARE(doc.state().state(), KraftDocState::State::Deleted);
    }

    void deleteDocFromInvalid()
    {
        KraftDoc doc;
        doc.state().setState(KraftDocState::State::Invalid);

        doc.slotDeleteDoc();
        QCOMPARE(doc.state().state(), KraftDocState::State::Deleted);
    }

    // A finalized document must never be deletable.
    void deleteDocFromFinalIsNoOp()
    {
        KraftDoc doc;
        doc.state().setState(KraftDocState::State::Final);

        doc.slotDeleteDoc();
        QCOMPARE(doc.state().state(), KraftDocState::State::Final);
    }

    void deleteDocFromDeletedIsNoOp()
    {
        KraftDoc doc;
        doc.state().setState(KraftDocState::State::Deleted);

        doc.slotDeleteDoc();
        QCOMPARE(doc.state().state(), KraftDocState::State::Deleted);
    }

    void deleteDocFromRetractedIsNoOp()
    {
        KraftDoc doc;
        doc.state().setState(KraftDocState::State::Retracted);

        doc.slotDeleteDoc();
        QCOMPARE(doc.state().state(), KraftDocState::State::Retracted);
    }

    void deleteDocFromConvertedIsNoOp()
    {
        KraftDoc doc;
        doc.state().setState(KraftDocState::State::Converted);

        doc.slotDeleteDoc();
        QCOMPARE(doc.state().state(), KraftDocState::State::Deleted);
    }

    void invalidateNeedsReason()
    {
        KraftDoc doc;
        doc.state().setState(KraftDocState::State::Final);
        QVERIFY(!doc.invalidate(QStringLiteral("  ")));
        QVERIFY(doc.state().is(KraftDocState::State::Final));
    }

    void invalidateOnlyFinal()
    {
        for (auto s : {KraftDocState::State::New, KraftDocState::State::Draft, KraftDocState::State::Retracted,
                       KraftDocState::State::Invalid, KraftDocState::State::Deleted, KraftDocState::State::Converted}) {
            KraftDoc doc;
            doc.state().setState(s);
            QVERIFY(!doc.invalidate(QStringLiteral("wrong price")));
            QVERIFY(doc.state().is(s));
            QVERIFY(doc.invalidReason().isEmpty());
        }
    }

    void invalidateAndCorrect()
    {
        KraftDoc doc;
        doc.setDocType(QStringLiteral("Rechnung")); // the index requires a type
        doc.setDate(QDate(2026, 1, 5));
        doc.setDueDate(QDate(2026, 1, 19));
        doc.setBuyerRef(QStringLiteral("ref-1"));
        QVERIFY(DocumentMan::self()->saveDocument(&doc)); // add to the index
        doc.setIdent(QStringLiteral("2026-042"));
        doc.state().setState(KraftDocState::State::Final);
        QVERIFY(DocumentMan::self()->saveDocument(&doc));

        // special chars and line breaks must survive in the XML attribute
        const QString reason = QStringLiteral("wrong price: 5 < 6 & \"x\"\nsecond line ä");
        QVERIFY(doc.invalidate(reason));
        QVERIFY(!doc.canBeDeleted());
        doc.slotDeleteDoc();

        // All needs to survive the reload
        QScopedPointer<KraftDoc> loaded(DocumentMan::self()->openDocumentByUuid(doc.uuid()));
        QVERIFY(loaded);
        QVERIFY(loaded->state().is(KraftDocState::State::Invalid));
        QCOMPARE(loaded->ident(), QStringLiteral("2026-042"));
        QCOMPARE(loaded->invalidReason(), reason);
        QVERIFY(loaded->invalidatedAt().isValid());

        QFile f(XmlIndexDb().xmlPathByUuid(doc.uuid()).filePath());
        QVERIFY(f.open(QIODevice::ReadOnly));
        QDomDocument dom;
        QVERIFY(dom.setContent(f.readAll()));
        const QDomElement meta = dom.documentElement().firstChildElement("meta");
        const QDomElement state = meta.firstChildElement("state");
        QCOMPARE(state.text(), KraftDocState::StateInvalidStr);
        QCOMPARE(state.attribute("reason"), reason);
        QVERIFY(QDateTime::fromString(state.attribute("invalidatedAt"), Qt::ISODate).isValid());
        QVERIFY(meta.firstChildElement("attrib").isNull());

        QScopedPointer<KraftDoc> draft(DocumentMan::self()->createCorrection(doc.uuid()));
        QVERIFY(draft);
        QVERIFY(draft->state().isNew());
        QVERIFY(draft->ident() != doc.ident());
        QVERIFY(draft->uuid() != doc.uuid());
        QVERIFY(draft->invalidReason().isEmpty());
        QVERIFY(!draft->invalidatedAt().isValid());
        QCOMPARE(draft->predecessor(), doc.uuid());
        QCOMPARE(draft->buyerRef(), QStringLiteral("ref-1"));
        QCOMPARE(draft->paymentDueDays(), 14);
    }

private:
    QTemporaryDir _dir;
};

QTEST_MAIN(T_KraftDocState)
#include "t_kraftdocstate.moc"
