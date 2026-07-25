#include "InterfaceRegistryTest.h"

#include <QTest>

#include "DBusConstants.h"
#include "dbus/InterfaceRegistry.h"

namespace dbus = constellar::dbus;

void InterfaceRegistryTest::parsesActivityInterface()
{
    InterfaceRegistry const registry;

    InterfaceDescriptor const* activity = registry.find(dbus::kActivityInterfaceName);
    QVERIFY(activity);
    QCOMPARE(activity->properties.size(), 5);

    QCOMPARE(activity->property(QStringLiteral("Type"))->signature, QStringLiteral("s"));
    QCOMPARE(activity->property(QStringLiteral("StartTime"))->signature, QStringLiteral("x"));
    QCOMPARE(activity->property(QStringLiteral("StopTime"))->signature, QStringLiteral("x"));
    QCOMPARE(activity->property(QStringLiteral("Outcome"))->signature, QStringLiteral("s"));
    QCOMPARE(activity->property(QStringLiteral("Recording"))->signature, QStringLiteral("o"));
    QVERIFY(activity->property(QStringLiteral("Type"))->readOnly);
}

void InterfaceRegistryTest::parsesEncounterInterface()
{
    InterfaceRegistry const registry;

    InterfaceDescriptor const* encounter = registry.find(dbus::kActivityEncounterInterfaceName);
    QVERIFY(encounter);
    QCOMPARE(encounter->properties.size(), 4);

    QCOMPARE(encounter->property(QStringLiteral("EncounterId"))->signature, QStringLiteral("u"));
    QCOMPARE(encounter->property(QStringLiteral("EncounterName"))->signature, QStringLiteral("s"));
    QCOMPARE(encounter->property(QStringLiteral("Difficulty"))->signature, QStringLiteral("s"));
    QCOMPARE(encounter->property(QStringLiteral("DifficultyId"))->signature, QStringLiteral("u"));
}

void InterfaceRegistryTest::parsesDungeonInterface()
{
    InterfaceRegistry const registry;

    InterfaceDescriptor const* dungeon = registry.find(dbus::kActivityDungeonInterfaceName);
    QVERIFY(dungeon);
    QCOMPARE(dungeon->properties.size(), 4);

    QCOMPARE(dungeon->property(QStringLiteral("ZoneId"))->signature, QStringLiteral("u"));
    QCOMPARE(dungeon->property(QStringLiteral("KeystoneLevel"))->signature, QStringLiteral("u"));
    QCOMPARE(dungeon->property(QStringLiteral("ChallengeMapId"))->signature, QStringLiteral("u"));
    QCOMPARE(dungeon->property(QStringLiteral("DurationMs"))->signature, QStringLiteral("x"));
}

void InterfaceRegistryTest::unknownInterfaceIsNull()
{
    InterfaceRegistry const registry;
    QVERIFY(!registry.find(QStringLiteral("dev.ulduar.Constellar1.NoSuchInterface")));
}

QTEST_MAIN(InterfaceRegistryTest)
