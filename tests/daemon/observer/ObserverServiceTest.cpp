#include "ObserverServiceTest.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "ActivityKeys.h"
#include "ObserverService.h"

namespace keys = constellar::keys;

namespace
{

QString timestampPrefix()
{
    return QStringLiteral("7/27/2024 21:39:13.0951  ");
}

void appendLine(QString const& path, QString const& line)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::Append | QIODevice::WriteOnly));
    file.write((line + QStringLiteral("\n")).toUtf8());
}

}  // namespace

void ObserverServiceTest::encounterStartPopulatesActivity()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ObserverService service(dir.path().toStdString());
    QVERIFY(service.start());

    appendLine(
        dir.filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );

    QTRY_VERIFY(!service.gameState().activity().isEmpty());
    QVariantMap const activity = service.gameState().activity();
    QCOMPARE(
        activity.value(QString::fromLatin1(keys::kType)), QString::fromLatin1(keys::kTypeEncounter)
    );
    QCOMPARE(
        activity.value(QString::fromLatin1(keys::kEncounterName)).toString(),
        QStringLiteral("Chimaerus the Undreamt God")
    );
    QCOMPARE(activity.value(QString::fromLatin1(keys::kDifficultyId)).toInt(), 15);
}

void ObserverServiceTest::mapChangePopulatesLocation()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ObserverService service(dir.path().toStdString());
    QVERIFY(service.start());

    appendLine(
        dir.filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("MAP_CHANGE,2537,\"Quel'Thalas\",10956.25,10152.08,-4002.08,-5208.33")
    );

    QTRY_VERIFY(!service.gameState().location().isEmpty());
    QVariantMap const location = service.gameState().location();
    QCOMPARE(location.value(QString::fromLatin1(keys::kUiMapId)).toInt(), 2537);
    QCOMPARE(
        location.value(QString::fromLatin1(keys::kUiMapName)).toString(),
        QStringLiteral("Quel'Thalas")
    );
}

void ObserverServiceTest::zoneChangePopulatesLocation()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ObserverService service(dir.path().toStdString());
    QVERIFY(service.start());

    appendLine(
        dir.filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() + QStringLiteral("ZONE_CHANGE,0,\"Sanctum of Light\",0")
    );

    QTRY_VERIFY(!service.gameState().location().isEmpty());
    QVariantMap const location = service.gameState().location();
    QCOMPARE(
        location.value(QString::fromLatin1(keys::kZoneName)).toString(),
        QStringLiteral("Sanctum of Light")
    );
    QVERIFY(!location.contains(QString::fromLatin1(keys::kUiMapId)));
}

void ObserverServiceTest::encounterEndClearsActivity()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ActivityTracker::Config config;
    config.raidOverrunSeconds = 1;
    ObserverService service(dir.path().toStdString(), config);
    QVERIFY(service.start());

    QString const path = dir.filePath(QStringLiteral("WoWCombatLog.txt"));
    appendLine(
        path, timestampPrefix() +
                  QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );
    QTRY_VERIFY(!service.gameState().activity().isEmpty());

    appendLine(
        path, timestampPrefix() +
                  QStringLiteral("ENCOUNTER_END,3306,\"Chimaerus the Undreamt God\",15,20,1")
    );

    QTRY_VERIFY_WITH_TIMEOUT(service.gameState().activity().isEmpty(), 3000);
}

void ObserverServiceTest::dungeonStartPopulatesActivity()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ObserverService service(dir.path().toStdString());
    QVERIFY(service.start());

    appendLine(
        dir.filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("CHALLENGE_MODE_START,\"Magisters' Terrace\",2811,558,10,[9]")
    );

    QTRY_VERIFY(!service.gameState().activity().isEmpty());
    QVariantMap const activity = service.gameState().activity();
    QCOMPARE(
        activity.value(QString::fromLatin1(keys::kType)), QString::fromLatin1(keys::kTypeDungeon)
    );
    QCOMPARE(activity.value(QString::fromLatin1(keys::kKeystoneLevel)).toInt(), 10);
}

QTEST_MAIN(ObserverServiceTest)
