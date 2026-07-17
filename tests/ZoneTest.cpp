#include "ZoneTest.h"

#include <QTest>

#include "ActivityKeys.h"
#include "Zone.h"

namespace keys = constellar::keys;

void ZoneTest::emptyMapIsEmpty()
{
    Zone const zone = Zone::fromVariantMap(QVariantMap{});

    QVERIFY(zone.isEmpty());
    QCOMPARE(zone.toString(), QStringLiteral("none"));
}

void ZoneTest::mapsNameAndMapId()
{
    QVariantMap const map{
        {keys::kZoneName, QStringLiteral("Eversong Woods")},
        {keys::kMapId, 2255u},
    };

    Zone const zone = Zone::fromVariantMap(map);

    QVERIFY(!zone.isEmpty());
    QCOMPARE(zone.name(), QStringLiteral("Eversong Woods"));
    QCOMPARE(zone.mapId(), 2255u);
    QCOMPARE(zone.toString(), QStringLiteral("Eversong Woods"));
}

QTEST_MAIN(ZoneTest)
