#include "ZoneTest.h"

#include <QTest>

#include "ActivityKeys.h"
#include "Zone.h"

namespace keys = constellar::keys;

void ZoneTest::emptyMapIsEmpty()
{
    Zone const zone = Zone::fromVariantMap(QVariantMap{});

    QVERIFY(zone.isEmpty());
    QCOMPARE(zone.toDisplayString(), QStringLiteral("none"));
}

void ZoneTest::mapsNameAndMapId()
{
    QVariantMap const map{
        {keys::kZoneName, QStringLiteral("Nerub-ar Palace")},
        {keys::kMapId, 2255u},
    };

    Zone const zone = Zone::fromVariantMap(map);

    QVERIFY(!zone.isEmpty());
    QCOMPARE(zone.name(), QStringLiteral("Nerub-ar Palace"));
    QCOMPARE(zone.mapId(), 2255u);
    QCOMPARE(zone.toDisplayString(), QStringLiteral("Nerub-ar Palace (mapId 2255)"));
}

QTEST_MAIN(ZoneTest)
