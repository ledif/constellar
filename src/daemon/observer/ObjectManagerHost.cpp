#include "ObjectManagerHost.h"

#include <QDBusError>
#include <QDebug>

#include "ActivityDBusAdaptor.h"
#include "ActivityDungeonDBusAdaptor.h"
#include "ActivityEncounterDBusAdaptor.h"
#include "ActivityProjection.h"
#include "CurrentActivityObject.h"
#include "DBusConstants.h"
#include "GameState.h"
#include "ManagedTypes.h"
#include "ObjectManagerAdaptor.h"
#include "ObserverService.h"

namespace dbus = constellar::dbus;

ObjectManagerHost::ObjectManagerHost(
    ObserverService* service, QDBusConnection connection, QObject* parent
)
    : QObject(parent), m_service(service), m_connection(std::move(connection))
{
    registerManagedTypes();

    m_objectManager = new ObjectManagerAdaptor(service, this);

    if (!m_connection.registerObject(QString::fromLatin1(dbus::kRootObjectPath), this))
    {
        qWarning() << "Failed to register the ObjectManager root at" << dbus::kRootObjectPath << ":"
                   << m_connection.lastError().message();
    }

    GameState const& gameState = m_service->gameState();
    connect(&gameState, &GameState::activityChanged, this, &ObjectManagerHost::onActivityChanged);
    connect(
        &gameState, &GameState::activityEnded, this,
        [this](QVariantMap const& /*endedActivity*/) { onActivityEnded(); }
    );
}

ObjectManagerHost::~ObjectManagerHost()
{
    if (m_currentActivity)
        m_connection.unregisterObject(QString::fromLatin1(dbus::kActivityObjectPath));
    m_connection.unregisterObject(QString::fromLatin1(dbus::kRootObjectPath));
}

void ObjectManagerHost::onActivityChanged(QVariantMap const& activity)
{
    // Already live (nothing to do), or this is GameState::endActivity's trailing
    // setActivity({}) clear -- handled by onActivityEnded instead.
    if (m_currentActivity || activity.isEmpty())
        return;

    m_currentActivity = std::make_unique<CurrentActivityObject>(activity);
    new ActivityDBusAdaptor(m_currentActivity.get());

    QVariantMapMap interfaces;
    interfaces[QString::fromLatin1(dbus::kActivityInterfaceName)] =
        constellar::observer::activityInterfaceProperties(activity);

    if (constellar::observer::isEncounter(activity))
    {
        new ActivityEncounterDBusAdaptor(m_currentActivity.get());
        interfaces[QString::fromLatin1(dbus::kActivityEncounterInterfaceName)] =
            constellar::observer::encounterInterfaceProperties(activity);
    }
    else if (constellar::observer::isDungeon(activity))
    {
        new ActivityDungeonDBusAdaptor(m_currentActivity.get());
        interfaces[QString::fromLatin1(dbus::kActivityDungeonInterfaceName)] =
            constellar::observer::dungeonInterfaceProperties(activity);
    }

    if (!m_connection.registerObject(
            QString::fromLatin1(dbus::kActivityObjectPath), m_currentActivity.get()
        ))
    {
        qWarning() << "Failed to register" << dbus::kActivityObjectPath << ":"
                   << m_connection.lastError().message();
    }

    m_objectManager->activityAppeared(interfaces);
}

void ObjectManagerHost::onActivityEnded()
{
    if (!m_currentActivity)
        return;

    // ActivityEnded (still-valid path) fires from ObserverDBusAdaptor's own
    // GameState::activityEnded connection, made before this one in main.cpp, so it
    // has already gone out by the time this handler runs. InterfacesRemoved next,
    // then the object goes away.
    m_objectManager->activityDisappeared();
    m_connection.unregisterObject(QString::fromLatin1(dbus::kActivityObjectPath));
    m_currentActivity.reset();
}
