#include "ActivityTest.h"

#include <QTest>

#include "Activity.h"
#include "ActivityKeys.h"

namespace keys = constellar::keys;

void ActivityTest::emptyMapIsNone()
{
    Activity const activity = Activity::fromVariantMap(QVariantMap{});

    QVERIFY(activity.isNone());
    QCOMPARE(activity.toString(), QStringLiteral("none"));
}

void ActivityTest::encounterMapsDifficultyAndName()
{
    QVariantMap const map{
        {keys::kType, QString::fromLatin1(keys::kTypeEncounter)},
        {keys::kDifficulty, QStringLiteral("Mythic")},
        {keys::kEncounterName, QStringLiteral("Ulgrax the Devourer")},
    };

    Activity const activity = Activity::fromVariantMap(map);

    QCOMPARE(activity.type(), Activity::Type::Encounter);
    QCOMPARE(activity.difficulty(), QStringLiteral("Mythic"));
    QCOMPARE(activity.encounterName(), QStringLiteral("Ulgrax the Devourer"));
    QCOMPARE(activity.toString(), QStringLiteral("Mythic Ulgrax the Devourer"));
}

void ActivityTest::dungeonMapsKeystoneLevel()
{
    QVariantMap const map{
        {keys::kType, QString::fromLatin1(keys::kTypeDungeon)},
        {keys::kKeystoneLevel, 18u},
    };

    Activity const activity = Activity::fromVariantMap(map);

    QCOMPARE(activity.type(), Activity::Type::Dungeon);
    QCOMPARE(activity.keystoneLevel(), 18u);
    QCOMPARE(activity.toString(), QStringLiteral("Mythic+ 18"));
}

void ActivityTest::unrecognizedTypeIsNone()
{
    QVariantMap const map{{keys::kType, QStringLiteral("something-future")}};

    Activity const activity = Activity::fromVariantMap(map);

    QVERIFY(activity.isNone());
    QCOMPARE(activity.toString(), QStringLiteral("none"));
}

QTEST_MAIN(ActivityTest)
