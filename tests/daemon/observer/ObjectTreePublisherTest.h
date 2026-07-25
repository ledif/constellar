#pragma once

#include <memory>

#include <QDBusConnection>
#include <QObject>
#include <QTemporaryDir>

#include "ObserverService.h"
#include "dbus/ObjectTreePublisher.h"
#include "dbus/adaptors/ObserverDBusAdaptor.h"

// Uses a real D-Bus connection with dbus-run-session
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

    void resolveReturnsNulloptWithNoActivity();
    void resolveReturnsActivityAndEncounterForEncounter();
    void resolveReturnsActivityAndDungeonForDungeon();

  private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<ObserverService> m_service;
    std::unique_ptr<ObserverDBusAdaptor> m_observerAdaptor;
    std::unique_ptr<ObjectTreePublisher> m_objectTreePublisher;
    QDBusConnection m_bus = QDBusConnection::sessionBus();
};
