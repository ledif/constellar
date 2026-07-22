#include "ObjectManagerHostTest.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QFile>
#include <QSignalSpy>
#include <QTest>

#include "DBusConstants.h"
#include "ManagedTypes.h"
#include "ObjectManagerAdaptor.h"
#include "ObjectManagerHost.h"
#include "ObserverDBusAdaptor.h"
#include "ObserverService.h"

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
    return QString::fromLatin1(dbus::kRootObjectPath);
}

QString activityPath()
{
    return QString::fromLatin1(dbus::kActivityObjectPath);
}

}  // namespace

ObjectManagerHostTest::~ObjectManagerHostTest() = default;

void ObjectManagerHostTest::init()
{
    m_bus = QDBusConnection::sessionBus();

    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());

    // Fast overrun timers -- the ordering/lifecycle tests don't want to sit
    // through the real 5s/20s daemon defaults.
    ActivityTracker::Config config;
    config.raidOverrunSeconds = 1;
    config.dungeonOverrunSeconds = 1;

    m_service = std::make_unique<ObserverService>(m_dir->path().toStdString(), config);
    m_observerAdaptor = std::make_unique<ObserverDBusAdaptor>(m_service.get(), m_bus);

    // Constructed after ObserverDBusAdaptor so its GameState::activityEnded
    // connection (and thus ActivityEnded) fires before ObjectManagerHost's
    // InterfacesRemoved -- the same ordering main.cpp relies on.
    m_objectManagerHost = std::make_unique<ObjectManagerHost>(m_service.get(), m_bus);

    QVERIFY(m_bus.registerObject(QString::fromLatin1(dbus::kObjectPath), m_service.get()));
    QVERIFY(m_bus.registerService(QString::fromLatin1(dbus::kServiceName)));
    QVERIFY(m_service->start());
}

void ObjectManagerHostTest::cleanup()
{
    m_bus.unregisterService(QString::fromLatin1(dbus::kServiceName));
    m_bus.unregisterObject(QString::fromLatin1(dbus::kObjectPath));

    m_objectManagerHost.reset();
    m_observerAdaptor.reset();
    m_service.reset();
    m_dir.reset();
}

void ObjectManagerHostTest::idleGetManagedObjectsHasOnlyObserver()
{
    QDBusInterface objectManager(
        QString::fromLatin1(dbus::kServiceName), rootPath(),
        QString::fromLatin1(dbus::kObjectManagerInterfaceName), m_bus
    );
    QVERIFY(objectManager.isValid());

    QDBusReply<DBusManagerStruct> const reply =
        objectManager.call(QStringLiteral("GetManagedObjects"));
    QVERIFY2(reply.isValid(), qPrintable(reply.error().message()));

    DBusManagerStruct const objects = reply.value();
    QVERIFY(objects.contains(QDBusObjectPath(QString::fromLatin1(dbus::kObjectPath))));
    QVERIFY(!objects.contains(QDBusObjectPath(activityPath())));
}

void ObjectManagerHostTest::encounterAppearsWithEncounterInterfaceOnly()
{
    appendLine(
        m_dir->filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    QDBusInterface objectManager(
        QString::fromLatin1(dbus::kServiceName), rootPath(),
        QString::fromLatin1(dbus::kObjectManagerInterfaceName), m_bus
    );
    QDBusReply<DBusManagerStruct> const reply =
        objectManager.call(QStringLiteral("GetManagedObjects"));
    QVERIFY2(reply.isValid(), qPrintable(reply.error().message()));

    QVariantMapMap const interfaces = reply.value().value(QDBusObjectPath(activityPath()));
    QVERIFY(interfaces.contains(QString::fromLatin1(dbus::kActivityInterfaceName)));
    QVERIFY(interfaces.contains(QString::fromLatin1(dbus::kActivityEncounterInterfaceName)));
    QVERIFY(!interfaces.contains(QString::fromLatin1(dbus::kActivityDungeonInterfaceName)));
}

void ObjectManagerHostTest::dungeonAppearsWithDungeonInterfaceOnly()
{
    appendLine(
        m_dir->filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("CHALLENGE_MODE_START,\"Magisters' Terrace\",2811,558,10,[9]")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    QDBusInterface objectManager(
        QString::fromLatin1(dbus::kServiceName), rootPath(),
        QString::fromLatin1(dbus::kObjectManagerInterfaceName), m_bus
    );
    QDBusReply<DBusManagerStruct> const reply =
        objectManager.call(QStringLiteral("GetManagedObjects"));
    QVERIFY2(reply.isValid(), qPrintable(reply.error().message()));

    QVariantMapMap const interfaces = reply.value().value(QDBusObjectPath(activityPath()));
    QVERIFY(interfaces.contains(QString::fromLatin1(dbus::kActivityInterfaceName)));
    QVERIFY(interfaces.contains(QString::fromLatin1(dbus::kActivityDungeonInterfaceName)));
    QVERIFY(!interfaces.contains(QString::fromLatin1(dbus::kActivityEncounterInterfaceName)));
}

void ObjectManagerHostTest::activityEndedFiresBeforeInterfacesRemoved()
{
    QString const logPath = m_dir->filePath(QStringLiteral("WoWCombatLog.txt"));
    appendLine(
        logPath,
        timestampPrefix() +
            QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    auto* objectManager = m_objectManagerHost->findChild<ObjectManagerAdaptor*>();
    QVERIFY(objectManager);

    QSignalSpy endedSpy(m_observerAdaptor.get(), &ObserverDBusAdaptor::ActivityEnded);
    QVERIFY(endedSpy.isValid());

    // Recorded the instant InterfacesRemoved fires -- proves ActivityEnded's
    // connected slots (made first in init()) already ran to completion, since
    // Qt invokes a signal's direct connections strictly in connection order.
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

void ObjectManagerHostTest::getManagedObjectsRoundTripsThroughWireWithLiveEncounter()
{
    appendLine(
        m_dir->filePath(QStringLiteral("WoWCombatLog.txt")),
        timestampPrefix() +
            QStringLiteral("ENCOUNTER_START,3306,\"Chimaerus the Undreamt God\",15,20,2549")
    );
    QTRY_VERIFY(!m_service->gameState().activity().isEmpty());

    QDBusInterface objectManager(
        QString::fromLatin1(dbus::kServiceName), rootPath(),
        QString::fromLatin1(dbus::kObjectManagerInterfaceName), m_bus
    );
    QDBusReply<DBusManagerStruct> const reply =
        objectManager.call(QStringLiteral("GetManagedObjects"));
    QVERIFY2(reply.isValid(), qPrintable(reply.error().message()));

    DBusManagerStruct const objects = reply.value();
    QVariantMap const activityProps = objects.value(QDBusObjectPath(activityPath()))
                                          .value(QString::fromLatin1(dbus::kActivityInterfaceName));
    QCOMPARE(activityProps.value(QStringLiteral("Type")).toString(), QStringLiteral("encounter"));
    QCOMPARE(
        activityProps.value(QStringLiteral("Recording")).value<QDBusObjectPath>(),
        QDBusObjectPath(QStringLiteral("/"))
    );

    QVariantMap const encounterProps =
        objects.value(QDBusObjectPath(activityPath()))
            .value(QString::fromLatin1(dbus::kActivityEncounterInterfaceName));
    QCOMPARE(encounterProps.value(QStringLiteral("EncounterId")).toUInt(), 3306u);
    QCOMPARE(
        encounterProps.value(QStringLiteral("EncounterName")).toString(),
        QStringLiteral("Chimaerus the Undreamt God")
    );
}

QTEST_MAIN(ObjectManagerHostTest)
