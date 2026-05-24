#include <QTest>
#include <QObject>

#include "kraftdoc.h"

class T_KraftDocState : public QObject {
    Q_OBJECT

private Q_SLOTS:
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

        // From Final: only Retracted. In particular, Deleted is NOT allowed.
        const auto fromFinal = KraftDocState::validFollowStates(KraftDocState::State::Final);
        QVERIFY(fromFinal.contains(KraftDocState::State::Retracted));
        QVERIFY(!fromFinal.contains(KraftDocState::State::Deleted));

        // From Invalid: may transition to Deleted.
        const auto fromInvalid = KraftDocState::validFollowStates(KraftDocState::State::Invalid);
        QVERIFY(fromInvalid.contains(KraftDocState::State::Deleted));

        // From Deleted: may be restored to Draft.
        const auto fromDeleted = KraftDocState::validFollowStates(KraftDocState::State::Deleted);
        QVERIFY(fromDeleted.contains(KraftDocState::State::Draft));
        QVERIFY(!fromDeleted.contains(KraftDocState::State::Deleted));

        // Terminal states: no follow states.
        QVERIFY(KraftDocState::validFollowStates(KraftDocState::State::Retracted).isEmpty());
        QVERIFY(KraftDocState::validFollowStates(KraftDocState::State::Converted).isEmpty());
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
        QCOMPARE(doc.state().state(), KraftDocState::State::Converted);
    }
};

QTEST_MAIN(T_KraftDocState)
#include "t_kraftdocstate.moc"
