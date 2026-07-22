#pragma once

#include <memory>

#include <QDBusConnection>
#include <QObject>
#include <QTemporaryDir>

#include "ObserverService.h"
#include "dbus/ObjectTreePublisher.h"
#include "dbus/adaptors/ObserverDBusAdaptor.h"

// Exercises the RFC-006/TASK-009 object tree over a *real* D-Bus connection (this
// binary is run under dbus-run-session -- see CMakeLists.txt's DBUS_SESSION), not
// just the in-process projection functions activityprojection_test covers. Proves
// the nested a{oa{sa{sv}}} GetManagedObjects marshalling actually round-trips, and
// that ActivityEnded/InterfacesRemoved fire in the order TASK-009's binding table
// requires.
class ObjectTreePublisherTest : public QObject
{
    Q_OBJECT

  public:
    ~ObjectTreePublisherTest() override;

  private Q_SLOTS:
    void init();
    void cleanup();

    void idleGetManagedObjectsHasOnlyObserver();
    void encounterAppearsWithEncounterInterfaceOnly();
    void dungeonAppearsWithDungeonInterfaceOnly();
    void activityEndedFiresBeforeInterfacesRemoved();
    void getManagedObjectsRoundTripsThroughWireWithLiveEncounter();

  private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<ObserverService> m_service;
    std::unique_ptr<ObserverDBusAdaptor> m_observerAdaptor;
    std::unique_ptr<ObjectTreePublisher> m_objectTreePublisher;
    QDBusConnection m_bus = QDBusConnection::sessionBus();
};
