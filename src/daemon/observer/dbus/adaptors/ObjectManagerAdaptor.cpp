#include "ObjectManagerAdaptor.h"

#include "GameState.h"
#include "ObserverService.h"
#include "dbus/ObjectTreePublisher.h"

using namespace Qt::StringLiterals;

namespace dbus = constellar::dbus;

ObjectManagerAdaptor::ObjectManagerAdaptor(
    ObserverService* service, ObjectTreePublisher* publisher, QObject* parent
)
    : QDBusAbstractAdaptor(parent), m_service(service), m_publisher(publisher)
{
    setAutoRelaySignals(true);
}

DBusManagerStruct ObjectManagerAdaptor::GetManagedObjects()
{
    DBusManagerStruct objects;

    QVariantMapMap observerInterfaces;
    observerInterfaces[dbus::kInterfaceName] = QVariantMap{
        {u"Activity"_s, m_service->gameState().activity()},
        {u"Location"_s, m_service->gameState().location()},
    };
    objects[QDBusObjectPath(dbus::kObjectPath)] = observerInterfaces;

    if (auto const snapshot = m_publisher->resolve(dbus::kActivityObjectPath); snapshot)
    {
        QVariantMapMap activityInterfaces;
        for (auto const& [interfaceName, props] : *snapshot)
            activityInterfaces[interfaceName] = props;

        objects[QDBusObjectPath(dbus::kActivityObjectPath)] = activityInterfaces;
    }

    return objects;
}

void ObjectManagerAdaptor::activityAppeared(QVariantMapMap const& interfaces)
{
    m_activityInterfaces = interfaces.keys();
    Q_EMIT InterfacesAdded(QDBusObjectPath(dbus::kActivityObjectPath), interfaces);
}

void ObjectManagerAdaptor::activityDisappeared()
{
    if (m_activityInterfaces.isEmpty())
        return;

    Q_EMIT InterfacesRemoved(QDBusObjectPath(dbus::kActivityObjectPath), m_activityInterfaces);

    m_activityInterfaces.clear();
}
