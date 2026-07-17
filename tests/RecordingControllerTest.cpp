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

QString challengeModeStartLine(const QString &hms, const QString &zoneName, int zoneId, int mapId,
                               int level) {
    return timestamp(hms) + QStringLiteral("  CHALLENGE_MODE_START,\"%1\",%2,%3,%4,[9]")
                                .arg(zoneName)
                                .arg(zoneId)
                                .arg(mapId)
                                .arg(level);
}

QString mapChangeLine(const QString &hms, int mapId, const QString &zoneName) {
    return timestamp(hms) +
           QStringLiteral("  MAP_CHANGE,%1,\"%2\",10956.25,10152.08,-4002.08,-5208.33")
               .arg(mapId)
               .arg(zoneName);
}

QString challengeModeEndLine(const QString &hms, int mapId, bool success, int level,
                             int durationMs) {
    return timestamp(hms) + QStringLiteral("  CHALLENGE_MODE_END,%1,%2,%3,%4,0.000000,0.000000")
                                .arg(mapId)
                                .arg(success ? 1 : 0)
                                .arg(level)
                                .arg(durationMs);
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

struct DungeonStarted {
    RecordingController::DungeonRun dungeon;
    QDateTime preRollFrom;
};

struct DungeonStopped {
    RecordingController::DungeonRun dungeon;
    bool success;
    int durationMs;
    QDateTime stopTime;
};

struct ZoneChanged {
    int mapId;
    QString zoneName;
};

// Collects signal emissions via plain lambdas rather than QSignalSpy, same
// reasoning as LogWatcherTest: the signal parameters (RaidEncounter,
// DungeonRun) aren't registered QMetaTypes.
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
        QObject::connect(
            &controller, &RecordingController::dungeonStarted,
            [this](const RecordingController::DungeonRun &dungeon, const QDateTime &preRollFrom) {
                dungeonStarted.append({dungeon, preRollFrom});
            });
        QObject::connect(&controller, &RecordingController::dungeonStopped,
                         [this](const RecordingController::DungeonRun &dungeon, bool success,
                                int durationMs, const QDateTime &stopTime) {
                             dungeonStopped.append({dungeon, success, durationMs, stopTime});
                         });
        QObject::connect(
            &controller, &RecordingController::zoneChanged,
            [this](int mapId, const QString &zoneName) { zoneChanges.append({mapId, zoneName}); });
    }

    QVector<Started> started;
    QVector<Stopped> stopped;
    QVector<DungeonStarted> dungeonStarted;
    QVector<DungeonStopped> dungeonStopped;
    QVector<ZoneChanged> zoneChanges;
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

void RecordingControllerTest::ignoresUnhandledLines() {
    RecordingController controller({});
    Collector collector(controller);

    controller.onLineReceived(LogLine(timestamp(QStringLiteral("21:40:06.0000")) +
                                      QStringLiteral("  ZONE_CHANGE,2549,\"Foo\",16")));

    QCOMPARE(collector.started.size(), 0);
    QCOMPARE(collector.stopped.size(), 0);
    QCOMPARE(collector.dungeonStarted.size(), 0);
    QCOMPARE(collector.dungeonStopped.size(), 0);
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

void RecordingControllerTest::dungeonStartsAboveKeystoneThreshold() {
    RecordingController controller({});
    Collector collector(controller);

    // Level 10 clears the default minKeystoneLevel of 2.
    controller.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("The Stonevault"), 2652, 501, 10)));

    QCOMPARE(collector.dungeonStarted.size(), 1);
    QCOMPARE(collector.dungeonStarted.at(0).dungeon.zoneId, 2652);
    QCOMPARE(collector.dungeonStarted.at(0).dungeon.mapId, 501);
    QCOMPARE(collector.dungeonStarted.at(0).dungeon.keystoneLevel, 10);
    QCOMPARE(collector.dungeonStarted.at(0).preRollFrom,
             collector.dungeonStarted.at(0).dungeon.startTime.addSecs(-15));
}

void RecordingControllerTest::dungeonSkipsBelowKeystoneThreshold() {
    RecordingController controller({});
    Collector collector(controller);

    // Level 1 falls below the default minKeystoneLevel of 2.
    controller.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("The Stonevault"), 2652, 501, 1)));

    QCOMPARE(collector.dungeonStarted.size(), 0);
}

void RecordingControllerTest::dungeonStopsAfterOverrunDelay() {
    RecordingController::Config config;
    config.dungeonOverrunSeconds = 1;
    RecordingController controller(config);
    Collector collector(controller);

    controller.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("The Stonevault"), 2652, 501, 10)));
    controller.onLineReceived(
        LogLine(challengeModeEndLine(QStringLiteral("22:10:00.0000"), 501, true, 10, 1800000)));

    QCOMPARE(collector.dungeonStopped.size(), 0);  // overrun hasn't elapsed yet
    QTRY_COMPARE_WITH_TIMEOUT(collector.dungeonStopped.size(), 1, 2500);
    QCOMPARE(collector.dungeonStopped.at(0).success, true);
    QCOMPARE(collector.dungeonStopped.at(0).durationMs, 1800000);
    QCOMPARE(collector.dungeonStopped.at(0).stopTime,
             collector.dungeonStarted.at(0).dungeon.startTime.addSecs(30 * 60 + 1));
}

void RecordingControllerTest::dungeonRepullDuringOverrunEndsPreviousImmediately() {
    RecordingController::Config config;
    config.dungeonOverrunSeconds = 5;  // long enough that a same-tick restart preempts it
    RecordingController controller(config);
    Collector collector(controller);

    controller.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("The Stonevault"), 2652, 501, 10)));
    controller.onLineReceived(
        LogLine(challengeModeEndLine(QStringLiteral("21:41:00.0000"), 501, false, 10, 60000)));

    // A new key starts before the 5s overrun tail would have elapsed.
    controller.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:41:05.0000"), QStringLiteral("The Stonevault"), 2652, 501, 12)));

    // The depleted key's stop is emitted immediately (pre-empted), not after 5s.
    QCOMPARE(collector.dungeonStopped.size(), 1);
    QCOMPARE(collector.dungeonStopped.at(0).success, false);
    QCOMPARE(collector.dungeonStarted.size(), 2);
    QCOMPARE(collector.dungeonStarted.at(1).dungeon.keystoneLevel, 12);
}

void RecordingControllerTest::dungeonSuppressesNestedEncounterSignals() {
    RecordingController controller({});
    Collector collector(controller);

    controller.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("The Stonevault"), 2652, 501, 10)));
    // A boss pull inside the key is a sub-segment, not a separate recording.
    controller.onLineReceived(LogLine(encounterStartLine(QStringLiteral("21:42:00.0000"), 2661,
                                                         QStringLiteral("Skarmorak"), 8, 501)));
    controller.onLineReceived(LogLine(encounterEndLine(QStringLiteral("21:45:00.0000"), 2661,
                                                       QStringLiteral("Skarmorak"), 8, true)));

    QCOMPARE(collector.started.size(), 0);
    QCOMPARE(collector.stopped.size(), 0);
    QCOMPARE(collector.dungeonStarted.size(), 1);
}

void RecordingControllerTest::dungeonIgnoresReStartWhileStillActive() {
    RecordingController controller({});
    Collector collector(controller);

    controller.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("The Stonevault"), 2652, 501, 10)));
    // Zoning out and back into the same still-active key re-fires START
    // without an intervening END.
    controller.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:41:00.0000"), QStringLiteral("The Stonevault"), 2652, 501, 10)));

    QCOMPARE(collector.dungeonStarted.size(), 1);
}

void RecordingControllerTest::mapChangeEmitsZoneChanged() {
    RecordingController controller({});
    Collector collector(controller);

    controller.onLineReceived(LogLine(mapChangeLine(QStringLiteral("18:57:18.8690"), 2533,
                                                    QStringLiteral("March on Quel'Danas"))));

    QCOMPARE(collector.zoneChanges.size(), 1);
    QCOMPARE(collector.zoneChanges.at(0).mapId, 2533);
    QCOMPARE(collector.zoneChanges.at(0).zoneName, QStringLiteral("March on Quel'Danas"));
}

QTEST_MAIN(RecordingControllerTest)
