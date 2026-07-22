#include "ObjectManagerAdaptor.h"

#include "ActivityProjection.h"
#include "GameState.h"
#include "ObserverService.h"

using namespace Qt::StringLiterals;

namespace dbus = constellar::dbus;

ObjectManagerAdaptor::ObjectManagerAdaptor(ObserverService* service, QObject* parent)
    : QDBusAbstractAdaptor(parent), m_service(service)
{
    setAutoRelaySignals(true);
}

DBusManagerStruct ObjectManagerAdaptor::GetManagedObjects()
{
    DBusManagerStruct objects;

    QVariantMapMap observerInterfaces;
    observerInterfaces[QString::fromLatin1(dbus::kInterfaceName)] = QVariantMap{
        {u"Activity"_s, m_service->gameState().activity()},
        {u"Location"_s, m_service->gameState().location()},
    };
    objects[QDBusObjectPath(QString::fromLatin1(dbus::kObjectPath))] = observerInterfaces;

    if (!m_activityInterfaces.isEmpty())
    {
        QVariantMap const bag = m_service->gameState().activity();

        QVariantMapMap activityInterfaces;
        activityInterfaces[QString::fromLatin1(dbus::kActivityInterfaceName)] =
            constellar::observer::activityInterfaceProperties(bag);

        if (constellar::observer::isEncounter(bag))
        {
            activityInterfaces[QString::fromLatin1(dbus::kActivityEncounterInterfaceName)] =
                constellar::observer::encounterInterfaceProperties(bag);
        }
        else if (constellar::observer::isDungeon(bag))
        {
            activityInterfaces[QString::fromLatin1(dbus::kActivityDungeonInterfaceName)] =
                constellar::observer::dungeonInterfaceProperties(bag);
        }

        objects[QDBusObjectPath(QString::fromLatin1(dbus::kActivityObjectPath))] =
            activityInterfaces;
    }

    return objects;
}

void ObjectManagerAdaptor::activityAppeared(QVariantMapMap const& interfaces)
{
    m_activityInterfaces = interfaces.keys();
    Q_EMIT InterfacesAdded(
        QDBusObjectPath(QString::fromLatin1(dbus::kActivityObjectPath)), interfaces
    );
}

void ObjectManagerAdaptor::activityDisappeared()
{
    if (m_activityInterfaces.isEmpty())
        return;

    Q_EMIT InterfacesRemoved(
        QDBusObjectPath(QString::fromLatin1(dbus::kActivityObjectPath)), m_activityInterfaces
    );
    m_activityInterfaces.clear();
}
