#include "ObjectTreePublisher.h"

#include <QDBusError>
#include <QDebug>

#include "ActivityProjection.h"
#include "DBusConstants.h"
#include "GameState.h"
#include "InterfaceRegistry.h"
#include "ManagedTypes.h"
#include "ObserverService.h"
#include "PropertyTreeObject.h"
#include "adaptors/ObjectManagerAdaptor.h"

namespace dbus = constellar::dbus;

ObjectTreePublisher::ObjectTreePublisher(
    ObserverService* service, QDBusConnection connection, QObject* parent
)
    : QObject(parent), m_service(service), m_connection(std::move(connection))
{
    registerManagedTypes();

    m_registry = std::make_unique<InterfaceRegistry>();

    m_objectManager = std::make_unique<ObjectManagerAdaptor>(service, this, this);

    if (!m_connection.registerObject(dbus::kRootObjectPath, this))
    {
        qWarning() << "Failed to register the ObjectManager root at" << dbus::kRootObjectPath << ":"
                   << m_connection.lastError().message();
    }

    m_propertyTree = std::make_unique<PropertyTreeObject>(*m_registry, *this, m_connection, this);

    if (!m_connection.registerVirtualObject(
            dbus::kActivitySubtreePath, m_propertyTree.get(), QDBusConnection::SubPath
        ))
    {
        qWarning() << "Failed to register the activity subtree at" << dbus::kActivitySubtreePath
                   << ":" << m_connection.lastError().message();
    }

    GameState const& gameState = m_service->gameState();
    connect(&gameState, &GameState::activityChanged, this, &ObjectTreePublisher::onActivityChanged);
    connect(
        &gameState, &GameState::activityEnded, this,
        [this](QVariantMap const& /*endedActivity*/) { onActivityEnded(); }
    );
}

ObjectTreePublisher::~ObjectTreePublisher()
{
    m_connection.unregisterObject(dbus::kActivitySubtreePath);
    m_connection.unregisterObject(dbus::kRootObjectPath);
}

std::optional<ObjectTreePublisher::ObjectSnapshot> ObjectTreePublisher::resolve(
    QStringView path
) const
{
    if (path != QStringView(dbus::kActivityObjectPath))
        return std::nullopt;

    QVariantMap const bag = m_service->gameState().activity();
    if (bag.isEmpty())
        return std::nullopt;

    ObjectSnapshot snapshot;
    snapshot.append(
        {dbus::kActivityInterfaceName, constellar::observer::activityInterfaceProperties(bag)}
    );

    if (constellar::observer::isEncounter(bag))
    {
        snapshot.append(
            {dbus::kActivityEncounterInterfaceName,
             constellar::observer::encounterInterfaceProperties(bag)}
        );
    }
    else if (constellar::observer::isDungeon(bag))
    {
        snapshot.append(
            {dbus::kActivityDungeonInterfaceName,
             constellar::observer::dungeonInterfaceProperties(bag)}
        );
    }

    return snapshot;
}

void ObjectTreePublisher::onActivityChanged(QVariantMap const& activity)
{
    // Already live (nothing to do), or this is GameState::endActivity's trailing
    // setActivity({}) clear -- handled by onActivityEnded instead.
    if (m_activityLive || activity.isEmpty())
        return;

    m_activityLive = true;

    auto const snapshot = resolve(dbus::kActivityObjectPath);
    Q_ASSERT(snapshot);  // just went live -- gameState().activity() can't be empty here.

    QVariantMapMap interfaces;
    for (auto const& [interfaceName, props] : *snapshot) interfaces[interfaceName] = props;

    m_objectManager->activityAppeared(interfaces);
}

void ObjectTreePublisher::onActivityEnded()
{
    if (!m_activityLive)
        return;

    // ActivityEnded (still-valid path) fires from ObserverDBusAdaptor's own
    // GameState::activityEnded connection, made before this one in main.cpp, so it
    // has already gone out by the time this handler runs. InterfacesRemoved next,
    // then the object goes away.
    m_objectManager->activityDisappeared();
    m_activityLive = false;
}
