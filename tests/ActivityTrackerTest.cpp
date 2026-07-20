#include "ActivityTrackerTest.h"

#include <QTest>
#include <QVector>

#include "ActivityTracker.h"
#include "LogLine.h"

namespace
{

QString timestamp(QString const& hms)
{
    return QStringLiteral("7/27/2024 ") + hms + QStringLiteral("-5");
}

QString encounterStartLine(
    QString const& hms, int encounterId, QString const& name, int difficultyId,
    int instanceId = 2549
)
{
    return timestamp(hms) + QStringLiteral("  ENCOUNTER_START,%1,\"%2\",%3,20,%4")
                                .arg(encounterId)
                                .arg(name)
                                .arg(difficultyId)
                                .arg(instanceId);
}

QString encounterEndLine(
    QString const& hms, int encounterId, QString const& name, int difficultyId, bool success
)
{
    return timestamp(hms) + QStringLiteral("  ENCOUNTER_END,%1,\"%2\",%3,20,%4")
                                .arg(encounterId)
                                .arg(name)
                                .arg(difficultyId)
                                .arg(success ? 1 : 0);
}

QString challengeModeStartLine(
    QString const& hms, QString const& zoneName, int zoneId, int mapId, int level
)
{
    return timestamp(hms) + QStringLiteral("  CHALLENGE_MODE_START,\"%1\",%2,%3,%4,[9]")
                                .arg(zoneName)
                                .arg(zoneId)
                                .arg(mapId)
                                .arg(level);
}

QString mapChangeLine(QString const& hms, int mapId, QString const& zoneName)
{
    return timestamp(hms) +
           QStringLiteral("  MAP_CHANGE,%1,\"%2\",10956.25,10152.08,-4002.08,-5208.33")
               .arg(mapId)
               .arg(zoneName);
}

QString challengeModeEndLine(QString const& hms, int mapId, bool success, int level, int durationMs)
{
    return timestamp(hms) + QStringLiteral("  CHALLENGE_MODE_END,%1,%2,%3,%4,0.000000,0.000000")
                                .arg(mapId)
                                .arg(success ? 1 : 0)
                                .arg(level)
                                .arg(durationMs);
}

struct Started
{
    RaidEncounter encounter;
    QDateTime preRollFrom;
};

struct Stopped
{
    RaidEncounter encounter;
    bool success;
    QDateTime stopTime;
};

struct DungeonStarted
{
    DungeonRun dungeon;
    QDateTime preRollFrom;
};

struct DungeonStopped
{
    DungeonRun dungeon;
    bool success;
    int durationMs;
    QDateTime stopTime;
};

struct ZoneChanged
{
    int mapId;
    QString zoneName;
};

// Collects signal emissions via plain lambdas rather than QSignalSpy, same
// reasoning as LogWatcherTest: the signal parameters (RaidEncounter,
// DungeonRun) aren't registered QMetaTypes.
struct Collector
{
    explicit Collector(ActivityTracker& tracker)
    {
        QObject::connect(
            &tracker, &ActivityTracker::encounterStarted,
            [this](RaidEncounter const& encounter, QDateTime const& preRollFrom)
            { started.append({encounter, preRollFrom}); }
        );
        QObject::connect(
            &tracker, &ActivityTracker::encounterStopped,
            [this](RaidEncounter const& encounter, bool success, QDateTime const& stopTime)
            { stopped.append({encounter, success, stopTime}); }
        );
        QObject::connect(
            &tracker, &ActivityTracker::dungeonStarted,
            [this](DungeonRun const& dungeon, QDateTime const& preRollFrom)
            { dungeonStarted.append({dungeon, preRollFrom}); }
        );
        QObject::connect(
            &tracker, &ActivityTracker::dungeonStopped,
            [this](
                DungeonRun const& dungeon, bool success, int durationMs, QDateTime const& stopTime
            ) { dungeonStopped.append({dungeon, success, durationMs, stopTime}); }
        );
        QObject::connect(
            &tracker, &ActivityTracker::zoneChanged,
            [this](int mapId, QString const& zoneName) { zoneChanges.append({mapId, zoneName}); }
        );
    }

    QVector<Started> started;
    QVector<Stopped> stopped;
    QVector<DungeonStarted> dungeonStarted;
    QVector<DungeonStopped> dungeonStopped;
    QVector<ZoneChanged> zoneChanges;
};

}  // namespace

void ActivityTrackerTest::startsRecordingAboveThreshold()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    // Heroic (15) clears the default minDifficulty of Normal.
    tracker.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 15
    )));

    QCOMPARE(collector.started.size(), 1);
    QCOMPARE(collector.started.at(0).encounter.encounterId, 3306);
    QCOMPARE(
        collector.started.at(0).encounter.encounterName,
        QStringLiteral("Chimaerus the Undreamt God")
    );
    QCOMPARE(collector.started.at(0).encounter.difficultyId, 15);
    QCOMPARE(
        collector.started.at(0).preRollFrom,
        collector.started.at(0).encounter.startTime.addSecs(-15)
    );
}

void ActivityTrackerTest::skipsBelowThreshold()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    // LFR (17) ranks below the default minDifficulty of Normal, despite
    // having a numerically larger difficultyID.
    tracker.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 17
    )));

    QCOMPARE(collector.started.size(), 0);
}

void ActivityTrackerTest::skipsUnknownDifficulty()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    // 8 isn't one of the four raid difficulty IDs (e.g. a M+ dungeon ID).
    tracker.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 8
    )));

    QCOMPARE(collector.started.size(), 0);
}

void ActivityTrackerTest::stopsAfterOverrunDelay()
{
    ActivityTracker::Config config;
    config.raidOverrunSeconds = 1;
    ActivityTracker tracker(config);
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 15
    )));
    tracker.onLineReceived(LogLine(encounterEndLine(
        QStringLiteral("21:52:31.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 15,
        true
    )));

    QCOMPARE(collector.stopped.size(), 0);  // overrun hasn't elapsed yet
    QTRY_COMPARE_WITH_TIMEOUT(collector.stopped.size(), 1, 2500);
    QCOMPARE(collector.stopped.at(0).success, true);
    QCOMPARE(
        collector.stopped.at(0).stopTime,
        collector.started.at(0).encounter.startTime.addSecs(12 * 60 + 26 + 1)
    );
}

void ActivityTrackerTest::repullDuringOverrunEndsPreviousImmediately()
{
    ActivityTracker::Config config;
    config.raidOverrunSeconds = 5;  // long enough that a same-tick repull preempts it
    ActivityTracker tracker(config);
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:00.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 15
    )));
    tracker.onLineReceived(LogLine(encounterEndLine(
        QStringLiteral("21:41:00.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 15,
        false
    )));

    // Re-pull the same boss before the 5s overrun tail would have elapsed.
    tracker.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:41:05.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 15
    )));

    // The wipe's stop is emitted immediately (pre-empted), not after 5s.
    QCOMPARE(collector.stopped.size(), 1);
    QCOMPARE(collector.stopped.at(0).success, false);
    QCOMPARE(collector.started.size(), 2);
}

void ActivityTrackerTest::ignoresUnhandledLines()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(
        timestamp(QStringLiteral("21:40:06.0000")) + QStringLiteral("  ZONE_CHANGE,2549,\"Foo\",16")
    ));

    QCOMPARE(collector.started.size(), 0);
    QCOMPARE(collector.stopped.size(), 0);
    QCOMPARE(collector.dungeonStarted.size(), 0);
    QCOMPARE(collector.dungeonStopped.size(), 0);
}

void ActivityTrackerTest::ignoresStrayEncounterEndWithoutStart()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(encounterEndLine(
        QStringLiteral("21:40:05.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 15,
        true
    )));

    QCOMPARE(collector.stopped.size(), 0);
}

void ActivityTrackerTest::ignoresMismatchedEncounterEnd()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:40:05.0000"), 3306, QStringLiteral("Chimaerus the Undreamt God"), 15
    )));
    // A END for a different encounterID shouldn't stop the one we're tracking.
    tracker.onLineReceived(LogLine(encounterEndLine(
        QStringLiteral("21:41:00.0000"), 9999, QStringLiteral("Someone Else"), 15, true
    )));

    QCOMPARE(collector.stopped.size(), 0);
}

void ActivityTrackerTest::dungeonStartsAboveKeystoneThreshold()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    // Level 10 clears the default minKeystoneLevel of 2.
    tracker.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("Magisters' Terrace"), 2811, 558, 10
    )));

    QCOMPARE(collector.dungeonStarted.size(), 1);
    QCOMPARE(collector.dungeonStarted.at(0).dungeon.zoneId, 2811);
    QCOMPARE(collector.dungeonStarted.at(0).dungeon.mapId, 558);
    QCOMPARE(collector.dungeonStarted.at(0).dungeon.keystoneLevel, 10);
    QCOMPARE(
        collector.dungeonStarted.at(0).preRollFrom,
        collector.dungeonStarted.at(0).dungeon.startTime.addSecs(-15)
    );
}

void ActivityTrackerTest::dungeonSkipsBelowKeystoneThreshold()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    // Level 1 falls below the default minKeystoneLevel of 2.
    tracker.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("Magisters' Terrace"), 2811, 558, 1
    )));

    QCOMPARE(collector.dungeonStarted.size(), 0);
}

void ActivityTrackerTest::dungeonStopsAfterOverrunDelay()
{
    ActivityTracker::Config config;
    config.dungeonOverrunSeconds = 1;
    ActivityTracker tracker(config);
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("Magisters' Terrace"), 2811, 558, 10
    )));
    tracker.onLineReceived(
        LogLine(challengeModeEndLine(QStringLiteral("22:10:00.0000"), 558, true, 10, 1800000))
    );

    QCOMPARE(collector.dungeonStopped.size(), 0);  // overrun hasn't elapsed yet
    QTRY_COMPARE_WITH_TIMEOUT(collector.dungeonStopped.size(), 1, 2500);
    QCOMPARE(collector.dungeonStopped.at(0).success, true);
    QCOMPARE(collector.dungeonStopped.at(0).durationMs, 1800000);
    QCOMPARE(
        collector.dungeonStopped.at(0).stopTime,
        collector.dungeonStarted.at(0).dungeon.startTime.addSecs(30 * 60 + 1)
    );
}

void ActivityTrackerTest::dungeonRepullDuringOverrunEndsPreviousImmediately()
{
    ActivityTracker::Config config;
    config.dungeonOverrunSeconds = 5;  // long enough that a same-tick restart preempts it
    ActivityTracker tracker(config);
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("Magisters' Terrace"), 2811, 558, 10
    )));
    tracker.onLineReceived(
        LogLine(challengeModeEndLine(QStringLiteral("21:41:00.0000"), 558, false, 10, 60000))
    );

    // A new key starts before the 5s overrun tail would have elapsed.
    tracker.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:41:05.0000"), QStringLiteral("Magisters' Terrace"), 2811, 558, 12
    )));

    // The depleted key's stop is emitted immediately (pre-empted), not after 5s.
    QCOMPARE(collector.dungeonStopped.size(), 1);
    QCOMPARE(collector.dungeonStopped.at(0).success, false);
    QCOMPARE(collector.dungeonStarted.size(), 2);
    QCOMPARE(collector.dungeonStarted.at(1).dungeon.keystoneLevel, 12);
}

void ActivityTrackerTest::dungeonSuppressesNestedEncounterSignals()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("Magisters' Terrace"), 2811, 558, 10
    )));
    // A boss pull inside the key is a sub-segment, not a separate recording.
    tracker.onLineReceived(LogLine(encounterStartLine(
        QStringLiteral("21:42:00.0000"), 3071, QStringLiteral("Arcanotron Custos"), 8, 2811
    )));
    tracker.onLineReceived(LogLine(encounterEndLine(
        QStringLiteral("21:45:00.0000"), 3071, QStringLiteral("Arcanotron Custos"), 8, true
    )));

    QCOMPARE(collector.started.size(), 0);
    QCOMPARE(collector.stopped.size(), 0);
    QCOMPARE(collector.dungeonStarted.size(), 1);
}

void ActivityTrackerTest::dungeonIgnoresReStartWhileStillActive()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    tracker.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:40:00.0000"), QStringLiteral("Magisters' Terrace"), 2811, 558, 10
    )));
    // Zoning out and back into the same still-active key re-fires START
    // without an intervening END.
    tracker.onLineReceived(LogLine(challengeModeStartLine(
        QStringLiteral("21:41:00.0000"), QStringLiteral("Magisters' Terrace"), 2811, 558, 10
    )));

    QCOMPARE(collector.dungeonStarted.size(), 1);
}

void ActivityTrackerTest::mapChangeEmitsZoneChanged()
{
    ActivityTracker tracker({});
    Collector collector(tracker);

    tracker.onLineReceived(
        LogLine(mapChangeLine(QStringLiteral("18:57:18.8690"), 2537, QStringLiteral("Quel'Thalas")))
    );

    QCOMPARE(collector.zoneChanges.size(), 1);
    QCOMPARE(collector.zoneChanges.at(0).mapId, 2537);
    QCOMPARE(collector.zoneChanges.at(0).zoneName, QStringLiteral("Quel'Thalas"));
}

QTEST_MAIN(ActivityTrackerTest)
