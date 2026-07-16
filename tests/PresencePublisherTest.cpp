#include "PresencePublisherTest.h"

#include <QJsonObject>
#include <QTest>

#include "PresencePublisher.h"

void PresencePublisherTest::encounterActivityMapsDifficultyAndName() {
    const QDateTime start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    const QJsonObject activity = PresencePublisher::encounterActivity(
        QStringLiteral("Ulgrax the Devourer"), QStringLiteral("Mythic"), start);

    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic Ulgrax the Devourer"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("Raid Encounter"));
    QCOMPARE(activity.value("timestamps").toObject().value("start").toInteger(),
             start.toUTC().toSecsSinceEpoch());
}

void PresencePublisherTest::dungeonActivityMapsKeystoneLevel() {
    const QDateTime start =
        QDateTime::fromString(QStringLiteral("2026-07-16T21:40:05Z"), Qt::ISODate);

    const QJsonObject activity = PresencePublisher::dungeonActivity(18, start);

    QCOMPARE(activity.value("details").toString(), QStringLiteral("Mythic+ Key +18"));
    QCOMPARE(activity.value("state").toString(), QStringLiteral("In a dungeon"));
    QCOMPARE(activity.value("timestamps").toObject().value("start").toInteger(),
             start.toUTC().toSecsSinceEpoch());
}

void PresencePublisherTest::idleActivityHasNoTimestamp() {
    const QJsonObject activity = PresencePublisher::idleActivity();

    QCOMPARE(activity.value("details").toString(), QStringLiteral("In World of Warcraft"));
    QVERIFY(!activity.contains("timestamps"));
}

QTEST_MAIN(PresencePublisherTest)
