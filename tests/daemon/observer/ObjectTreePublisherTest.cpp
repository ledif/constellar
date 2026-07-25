#include "ObjectTreePublisherTest.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QFile>
#include <QSignalSpy>
#include <QTest>

#include "DBusConstants.h"
#include "ObserverService.h"
#include "dbus/ManagedTypes.h"
#include "dbus/ObjectTreePublisher.h"
#include "dbus/adaptors/ObjectManagerAdaptor.h"
#include "dbus/adaptors/ObserverDBusAdaptor.h"

namespace dbus = constellar::dbus;

namespace
{

QString timestampPrefix()
{
    return QStringLiteral("7/27/2024 21:39:13.0951  ");
}

void appendLine(QString const& path, QString const& line)
{
    QFile file(path);
    QVERIFY2(file.open(QIODevice::Append | QIODevice::WriteOnly), "failed to open log fixture");
    file.write((line + QStringLiteral("\n")).toUtf8());
}

QString rootPath()
{
    return dbus::kRootObjectPath;
}

QString activityPath()
{
    return dbus::kActivityObjectPath;
}

}  // namespace

ObjectTreePublisherTest::~ObjectTreePublisherTest() = default;

void ObjectTreePublisherTest::init()
{
    m_bus = QDBusConnection::sessionBus();

    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    // Fast overrun timers
    ActivityTracker::Config config;
    config.raidOverrunSeconds = 1;
    config.dungeonOverrunSeconds = 1;

    m_service = std::make_unique<ObserverService>(m_dir->path().toStdString(), config);
    m_observerAdaptor = std::make_unique<ObserverDBusAdaptor>(m_service.get(), m_bus);

    m_objectTreePublisher = std::make_unique<ObjectTreePublisher>(m_service.get(), m_bus);

    QVERIFY(m_bus.registerObject(dbus::kObjectPath, m_service.get()));
    QVERIFY(m_bus.registerService(dbus::kServiceName));
    QVERIFY(m_service->start());
}

void ObjectTreePublisherTest::cleanup()
{
    m_bus.unregisterService(dbus::kServiceName);
    m_bus.unregisterObject(dbus::kObjectPath);

    m_objectTreePublisher.reset();
    m_observerAdaptor.reset();
    m_service.reset();
    m_dir.reset();
}

void ObjectTreePublisherTest::idleGetManagedObjectsHasOnlyObserver()
{
    QDBusInterface objectManager(
        dbus::kServiceName, rootPath(), dbus::kObjectManagerInterfaceName, m_bus
    );
    QVERIFY(objectManager.isValid());

    QDBusReply<DBusManagerStruct> const reply =
        objectManager.call(QStringLiteral("GetManagedObjects"));
    QVERIFY2(reply.isValid(), qPrintable(reply.error().message()));

    DBusManagerStruct const objects = reply.value();
    QVERIFY(objects.contains(QDBusObjectPath(dbus::kObjectPath)));
    QVERIFY(!objects.contains(QDBusObjectPath(activityPath())));
}

void ObjectTreePublisherTest::encounterAppearsWithEncounterInterfaceOnly()
{
    appendLine(
        m_dir->filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    QDBusInterface objectManager(
        dbus::kServiceName, rootPath(), dbus::kObjectManagerInterfaceName, m_bus
    );
    QDBusReply<DBusManagerStruct> const reply =
        objectManager.call(QStringLiteral("GetManagedObjects"));
    QVERIFY2(reply.isValid(), qPrintable(reply.error().message()));

    QVariantMapMap const interfaces = reply.value().value(QDBusObjectPath(activityPath()));
    QVERIFY(interfaces.contains(dbus::kActivityInterfaceName));
    QVERIFY(interfaces.contains(dbus::kActivityEncounterInterfaceName));
    QVERIFY(!interfaces.contains(dbus::kActivityDungeonInterfaceName));
}

void ObjectTreePublisherTest::dungeonAppearsWithDungeonInterfaceOnly()
{
    appendLine(
        m_dir->filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("CHALLENGE_MODE_START,\"Magisters' Terrace\",2811,558,10,[9]")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    QDBusInterface objectManager(
        dbus::kServiceName, rootPath(), dbus::kObjectManagerInterfaceName, m_bus
    );
    QDBusReply<DBusManagerStruct> const reply =
        objectManager.call(QStringLiteral("GetManagedObjects"));
    QVERIFY2(reply.isValid(), qPrintable(reply.error().message()));

    QVariantMapMap const interfaces = reply.value().value(QDBusObjectPath(activityPath()));
    QVERIFY(interfaces.contains(dbus::kActivityInterfaceName));
    QVERIFY(interfaces.contains(dbus::kActivityDungeonInterfaceName));
    QVERIFY(!interfaces.contains(dbus::kActivityEncounterInterfaceName));
}

void ObjectTreePublisherTest::activityEndedFiresBeforeInterfacesRemoved()
{
    QString const logPath = m_dir->filePath(QStringLiteral("WoWCombatLog.txt"));
    appendLine(
        logPath,
        timestampPrefix() +
            QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    auto* objectManager = m_objectTreePublisher->findChild<ObjectManagerAdaptor*>();
    QVERIFY(objectManager);

    QSignalSpy endedSpy(m_observerAdaptor.get(), &ObserverDBusAdaptor::ActivityEnded);
    QVERIFY(endedSpy.isValid());

    // Updatee when InterfacesRemoved fires
    int endedCountWhenRemoved = -1;
    connect(
        objectManager, &ObjectManagerAdaptor::InterfacesRemoved, this,
        [&](QDBusObjectPath const& /*object*/, QStringList const& /*interfaces*/)
        { endedCountWhenRemoved = endedSpy.count(); }
    );

    appendLine(
        logPath, timestampPrefix() +
                     QStringLiteral("ENCOUNTER_END,3306,\"Chimaerus the Undreamt God\",15,20,1")
    );
    QTRY_VERIFY_WITH_TIMEOUT(m_service->gameState().activity().isEmpty(), 3000);

    QCOMPARE(endedCountWhenRemoved, 1);
    QCOMPARE(endedSpy.count(), 1);

    QList<QVariant> const endedArgs = endedSpy.constFirst();
    QCOMPARE(endedArgs.at(0).value<QDBusObjectPath>(), QDBusObjectPath(activityPath()));
}

void ObjectTreePublisherTest::getManagedObjectsRoundTripsThroughWireWithLiveEncounter()
{
    appendLine(
        m_dir->filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    QDBusInterface objectManager(
        dbus::kServiceName, rootPath(), dbus::kObjectManagerInterfaceName, m_bus
    );
    QDBusReply<DBusManagerStruct> const reply =
        objectManager.call(QStringLiteral("GetManagedObjects"));
    QVERIFY2(reply.isValid(), qPrintable(reply.error().message()));

    DBusManagerStruct const objects = reply.value();
    QVariantMap const activityProps =
        objects.value(QDBusObjectPath(activityPath())).value(dbus::kActivityInterfaceName);
    QCOMPARE(activityProps.value(QStringLiteral("Type")).toString(), QStringLiteral("encounter"));
    QCOMPARE(
        activityProps.value(QStringLiteral("Recording")).value<QDBusObjectPath>(),
        QDBusObjectPath(QStringLiteral("/"))
    );

    QVariantMap const encounterProps =
        objects.value(QDBusObjectPath(activityPath())).value(dbus::kActivityEncounterInterfaceName);
    QCOMPARE(encounterProps.value(QStringLiteral("EncounterId")).toUInt(), 3306u);
    QCOMPARE(
        encounterProps.value(QStringLiteral("EncounterName")).toString(),
        QStringLiteral("Chimaerus the Undreamt God")
    );
}

void ObjectTreePublisherTest::resolveReturnsNulloptWithNoActivity()
{
    QVERIFY(!m_objectTreePublisher->resolve(dbus::kActivityObjectPath));
}

void ObjectTreePublisherTest::resolveReturnsActivityAndEncounterForEncounter()
{
    appendLine(
        m_dir->filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    auto const snapshot = m_objectTreePublisher->resolve(dbus::kActivityObjectPath);
    QVERIFY(snapshot);
    QCOMPARE(snapshot->size(), 2);
    QCOMPARE(snapshot->at(0).first, dbus::kActivityInterfaceName);
    QCOMPARE(snapshot->at(1).first, dbus::kActivityEncounterInterfaceName);
}

void ObjectTreePublisherTest::resolveReturnsActivityAndDungeonForDungeon()
{
    appendLine(
        m_dir->filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("CHALLENGE_MODE_START,\"Magisters' Terrace\",2811,558,10,[9]")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    auto const snapshot = m_objectTreePublisher->resolve(dbus::kActivityObjectPath);
    QVERIFY(snapshot);
    QCOMPARE(snapshot->size(), 2);
    QCOMPARE(snapshot->at(0).first, dbus::kActivityInterfaceName);
    QCOMPARE(snapshot->at(1).first, dbus::kActivityDungeonInterfaceName);
}

QTEST_MAIN(ObjectTreePublisherTest)
