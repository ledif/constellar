#include "LocationTest.h"

#include <QTest>

#include "ActivityKeys.h"
#include "Location.h"

namespace keys = constellar::keys;

void LocationTest::emptyLocationIsEmpty()
{
    Location const location = Location::fromVariantMap(QVariantMap{});

    QVERIFY(location.isEmpty());
    QVERIFY(!location.uiMap());
    QVERIFY(!location.zone());
    QVERIFY(location.toVariantMap().isEmpty());
}

void LocationTest::uiMapOnlyRoundTrips()
{
    Location location;
    location.setUiMap(UiMap{2413, QStringLiteral("Harandar"), {}});

    QVariantMap const map = location.toVariantMap();
    QCOMPARE(map.value(QString::fromLatin1(keys::kUiMapId)).toUInt(), 2413u);
    QCOMPARE(
        map.value(QString::fromLatin1(keys::kUiMapName)).toString(), QStringLiteral("Harandar")
    );
    QVERIFY(!map.contains(QString::fromLatin1(keys::kUiMapBounds)));
    QVERIFY(!map.contains(QString::fromLatin1(keys::kZoneInstanceId)));

    Location const roundTripped = Location::fromVariantMap(map);
    QVERIFY(roundTripped.uiMap());
    QCOMPARE(roundTripped.uiMap()->id, 2413u);
    QCOMPARE(roundTripped.uiMap()->name, QStringLiteral("Harandar"));
    QVERIFY(!roundTripped.zone());
}

void LocationTest::uiMapWithBoundsRoundTrips()
{
    Location location;
    MapBounds const bounds{2527.08, -2520.83, 3579.16, -3993.75};
    location.setUiMap(UiMap{2413, QStringLiteral("Harandar"), bounds});

    QVariantMap const map = location.toVariantMap();
    QVERIFY(map.contains(QString::fromLatin1(keys::kUiMapBounds)));

    Location const roundTripped = Location::fromVariantMap(map);
    QVERIFY(roundTripped.uiMap());
    QCOMPARE(roundTripped.uiMap()->bounds.x0, bounds.x0);
    QCOMPARE(roundTripped.uiMap()->bounds.x1, bounds.x1);
    QCOMPARE(roundTripped.uiMap()->bounds.y0, bounds.y0);
    QCOMPARE(roundTripped.uiMap()->bounds.y1, bounds.y1);
}

void LocationTest::uiMapWithInvalidBoundsOmitsBoundsKey()
{
    Location location;
    location.setUiMap(UiMap{2413, QStringLiteral("Harandar"), MapBounds{}});

    QVariantMap const map = location.toVariantMap();
    QVERIFY(!map.contains(QString::fromLatin1(keys::kUiMapBounds)));
}

void LocationTest::zoneOnlyRoundTrips()
{
    Location location;
    location.setZone(Zone{2694, QStringLiteral("Harandar"), 0});

    QVariantMap const map = location.toVariantMap();
    QCOMPARE(map.value(QString::fromLatin1(keys::kZoneInstanceId)).toUInt(), 2694u);
    QCOMPARE(
        map.value(QString::fromLatin1(keys::kZoneName)).toString(), QStringLiteral("Harandar")
    );
    QVERIFY(!map.contains(QString::fromLatin1(keys::kUiMapId)));

    Location const roundTripped = Location::fromVariantMap(map);
    QVERIFY(roundTripped.zone());
    QCOMPARE(roundTripped.zone()->instanceId, 2694u);
    QCOMPARE(roundTripped.zone()->name, QStringLiteral("Harandar"));
    QVERIFY(!roundTripped.uiMap());
}

void LocationTest::bothHalvesRoundTrip()
{
    Location location;
    location.setUiMap(UiMap{2393, QStringLiteral("Silvermoon City"), {}});
    location.setZone(Zone{0, QStringLiteral("Sanctum of Light"), 0});

    Location const roundTripped = Location::fromVariantMap(location.toVariantMap());
    QVERIFY(roundTripped.uiMap());
    QVERIFY(roundTripped.zone());
    QCOMPARE(roundTripped.uiMap()->name, QStringLiteral("Silvermoon City"));
    QCOMPARE(roundTripped.zone()->name, QStringLiteral("Sanctum of Light"));
}

void LocationTest::displayNamePrefersZoneOverUiMap()
{
    Location location;
    location.setUiMap(UiMap{2393, QStringLiteral("Silvermoon City"), {}});
    location.setZone(Zone{0, QStringLiteral("Sanctum of Light"), 0});

    QCOMPARE(location.displayName(), QStringLiteral("Sanctum of Light"));
}

void LocationTest::displayNameFallsBackToUiMapWhenZoneNameEmpty()
{
    Location location;
    location.setUiMap(UiMap{2393, QStringLiteral("Silvermoon City"), {}});

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
