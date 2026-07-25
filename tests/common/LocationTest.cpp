#include "LocationTest.h"

#include <QTest>

#include "ActivityKeys.h"
#include "Location.h"

namespace keys = constellar::keys;

namespace
{

QString keyStr(char const* key)
{
    return QString::fromLatin1(key);
}

}  // namespace

void LocationTest::emptyLocationIsEmpty()
{
    Location const location = Location::fromVariantMap(QVariantMap{});

    QVERIFY(location.isEmpty());
    QVERIFY(!location.uiMap());
    QVERIFY(!location.zone());
    QVERIFY(location.toVariantMap().isEmpty());
}

void LocationTest::uiMapEmitsWireKeys()
{
    Location location;
    location.setUiMap(UiMap{2413, QStringLiteral("Harandar")});

    QVariantMap const map = location.toVariantMap();

    // Resolved name falls back to the uiMap name when category is unknown with no zone.
    QCOMPARE(map.value(keyStr(keys::kZoneName)).toString(), QStringLiteral("Harandar"));
    QCOMPARE(map.value(keyStr(keys::kZoneCategory)).toString(), QStringLiteral("unknown"));
    QCOMPARE(map.value(keyStr(keys::kUiMapId)).toUInt(), 2413u);
}

void LocationTest::zoneEmitsCategory()
{
    Location location;
    location.setZone(Zone{QStringLiteral("Windrunner Spire"), 23});  // Dungeon Mythic

    QVariantMap const map = location.toVariantMap();
    QCOMPARE(map.value(keyStr(keys::kZoneName)).toString(), QStringLiteral("Windrunner Spire"));
    QCOMPARE(map.value(keyStr(keys::kZoneCategory)).toString(), QStringLiteral("dungeon"));
    QCOMPARE(map.value(keyStr(keys::kUiMapId)).toUInt(), 0u);
}

void LocationTest::zoneCategoryMappings()
{
    auto categoryOf = [](quint32 difficultyId)
    {
        Location location;
        location.setZone(Zone{QStringLiteral("Somewhere"), difficultyId});
        return location.toVariantMap().value(keyStr(keys::kZoneCategory)).toString();
    };

    QCOMPARE(categoryOf(0), QStringLiteral("open-world"));
    QCOMPARE(categoryOf(1), QStringLiteral("dungeon"));
    QCOMPARE(categoryOf(24), QStringLiteral("dungeon"));
    QCOMPARE(categoryOf(14), QStringLiteral("raid"));
    QCOMPARE(categoryOf(17), QStringLiteral("raid"));
    QCOMPARE(categoryOf(208), QStringLiteral("delve"));
    QCOMPARE(categoryOf(999), QStringLiteral("unknown"));
}

void LocationTest::locationRoundTripsThroughWire()
{
    Location location;
    location.setUiMap(UiMap{2393, QStringLiteral("Silvermoon City")});
    location.setZone(Zone{QStringLiteral("Sanctum of Light"), 0});

    Location const roundTripped = Location::fromVariantMap(location.toVariantMap());

    // The resolved display name and the stable map id survives
    QCOMPARE(roundTripped.displayName(), QStringLiteral("Sanctum of Light"));
    QVERIFY(roundTripped.uiMap());
    QCOMPARE(roundTripped.uiMap()->id, 2393u);
}

void LocationTest::displayNamePrefersZoneOverUiMap()
{
    Location location;
    location.setUiMap(UiMap{2393, QStringLiteral("Silvermoon City")});
    location.setZone(Zone{QStringLiteral("Sanctum of Light"), 0});

    QCOMPARE(location.displayName(), QStringLiteral("Sanctum of Light"));
}

void LocationTest::displayNameFallsBackToUiMapWhenZoneNameEmpty()
{
    Location location;
    location.setUiMap(UiMap{2393, QStringLiteral("Silvermoon City")});

    QCOMPARE(location.displayName(), QStringLiteral("Silvermoon City"));
}

void LocationTest::displayNameEmptyWhenNeitherSet()
{
    Location const location;

    QVERIFY(location.displayName().isEmpty());
}

void LocationTest::toStringIsNoneWhenEmpty()
{
    Location const location;

    QCOMPARE(location.toString(), QStringLiteral("none"));
}

QTEST_MAIN(LocationTest)
