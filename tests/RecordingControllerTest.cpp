#include "RecordingControllerTest.h"

#include <QTest>
#include <QVector>

#include "LogLine.h"
#include "RecordingController.h"

namespace {

QString timestamp(const QString &hms) {
    return QStringLiteral("7/27/2024 ") + hms + QStringLiteral("-5");
}

QString encounterStartLine(const QString &hms, int encounterId, const QString &name,
                           int difficultyId, int instanceId = 2549) {
    return timestamp(hms) + QStringLiteral("  ENCOUNTER_START,%1,\"%2\",%3,20,%4")
                                .arg(encounterId)
                                .arg(name)
                                .arg(difficultyId)
                                .arg(instanceId);
}

QString encounterEndLine(const QString &hms, int encounterId, const QString &name, int difficultyId,
                         bool success) {
    return timestamp(hms) + QStringLiteral("  ENCOUNTER_END,%1,\"%2\",%3,20,%4")
                                .arg(encounterId)
                                .arg(name)
                                .arg(difficultyId)
                                .arg(success ? 1 : 0);
}

struct Started {
    RecordingController::RaidEncounter encounter;
    QDateTime preRollFrom;
};

struct Stopped {
    RecordingController::RaidEncounter encounter;
    bool success;
    QDateTime stopTime;
};

// Collects signal emissions via plain lambdas rather than QSignalSpy, same
// reasoning as LogWatcherTest: the signal parameters (RaidEncounter) aren't
// registered QMetaTypes.
struct Collector {
    explicit Collector(RecordingController &controller) {
        QObject::connect(
            &controller, &RecordingController::recordingStarted,
            [this](const RecordingController::RaidEncounter &encounter,
                   const QDateTime &preRollFrom) { started.append({encounter, preRollFrom}); });
        QObject::connect(
            &controller, &RecordingController::recordingStopped,
            [this](const RecordingController::RaidEncounter &encounter, bool success,
                   const QDateTime &stopTime) { stopped.append({encounter, success, stopTime}); });
    }

    QVector<Started> started;
    QVector<Stopped> stopped;
};

}  // namespace

void RecordingControllerTest::startsRecordingAboveThreshold() {
    RecordingController controller({});
    Collector collector(controller);

    // Heroic (15) clears the default minDifficulty of Normal.
    controller.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 15)));

    QCOMPARE(collector.started.size(), 1);
    QCOMPARE(collector.started.at(0).encounter.encounterId, 2820);
    QCOMPARE(collector.started.at(0).encounter.encounterName, QStringLiteral("Fyrakk the Blazing"));
    QCOMPARE(collector.started.at(0).encounter.difficultyId, 15);
    QCOMPARE(collector.started.at(0).preRollFrom,
             collector.started.at(0).encounter.startTime.addSecs(-15));
}

void RecordingControllerTest::skipsBelowThreshold() {
    RecordingController controller({});
    Collector collector(controller);

    // LFR (17) ranks below the default minDifficulty of Normal, despite
    // having a numerically larger difficultyID.
    controller.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 17)));

    QCOMPARE(collector.started.size(), 0);
}

void RecordingControllerTest::skipsUnknownDifficulty() {
    RecordingController controller({});
    Collector collector(controller);

    // 8 isn't one of the four raid difficulty IDs (e.g. a M+ dungeon ID).
    controller.onLineReceived(LogLine(encounterStartLine(QStringLiteral("21:40:05.0000"), 2820,
                                                         QStringLiteral("Fyrakk the Blazing"), 8)));

    QCOMPARE(collector.started.size(), 0);
}

void RecordingControllerTest::stopsAfterOverrunDelay() {
    RecordingController::Config config;
    config.raidOverrunSeconds = 1;
    RecordingController controller(config);
    Collector collector(controller);

    controller.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 15)));
    controller.onLineReceived(LogLine(encounterEndLine(
        QStringLiteral("21:52:31.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 15, true)));

    QCOMPARE(collector.stopped.size(), 0);  // overrun hasn't elapsed yet
    QTRY_COMPARE_WITH_TIMEOUT(collector.stopped.size(), 1, 2500);
    QCOMPARE(collector.stopped.at(0).success, true);
    QCOMPARE(collector.stopped.at(0).stopTime,
             collector.started.at(0).encounter.startTime.addSecs(12 * 60 + 26 + 1));
}

void RecordingControllerTest::repullDuringOverrunEndsPreviousImmediately() {
    RecordingController::Config config;
    config.raidOverrunSeconds = 5;  // long enough that a same-tick repull preempts it
    RecordingController controller(config);
    Collector collector(controller);

    controller.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:00.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 15)));
    controller.onLineReceived(LogLine(encounterEndLine(
        QStringLiteral("21:41:00.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 15, false)));

    // Re-pull the same boss before the 5s overrun tail would have elapsed.
    controller.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:41:05.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 15)));

    // The wipe's stop is emitted immediately (pre-empted), not after 5s.
    QCOMPARE(collector.stopped.size(), 1);
    QCOMPARE(collector.stopped.at(0).success, false);
    QCOMPARE(collector.started.size(), 2);
}

void RecordingControllerTest::ignoresNonRaidLines() {
    RecordingController controller({});
    Collector collector(controller);

    controller.onLineReceived(
        LogLine(timestamp(QStringLiteral("21:40:05.0000")) +
                QStringLiteral("  CHALLENGE_MODE_START,\"Ruby Life Pools\",2451,504,10,[9]")));
    controller.onLineReceived(LogLine(timestamp(QStringLiteral("21:40:06.0000")) +
                                      QStringLiteral("  ZONE_CHANGE,2549,\"Foo\",16")));

    QCOMPARE(collector.started.size(), 0);
    QCOMPARE(collector.stopped.size(), 0);
}

void RecordingControllerTest::ignoresStrayEncounterEndWithoutStart() {
    RecordingController controller({});
    Collector collector(controller);

    controller.onLineReceived(LogLine(encounterEndLine(
        QStringLiteral("21:40:05.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 15, true)));

    QCOMPARE(collector.stopped.size(), 0);
}

void RecordingControllerTest::ignoresMismatchedEncounterEnd() {
    RecordingController controller({});
    Collector collector(controller);

    controller.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 2820, QStringLiteral("Fyrakk the Blazing"), 15)));
    // A END for a different encounterID shouldn't stop the one we're tracking.
    controller.onLineReceived(LogLine(encounterEndLine(QStringLiteral("21:41:00.0000"), 9999,
                                                       QStringLiteral("Someone Else"), 15, true)));

    QCOMPARE(collector.stopped.size(), 0);
}

QTEST_MAIN(RecordingControllerTest)
