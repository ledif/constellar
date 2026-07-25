#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QStringList>

#include "DBusConstants.h"
#include "dbus/ManagedTypes.h"

class ObserverService;
class ObjectTreePublisher;

// The standard org.freedesktop.DBus.ObjectManagerAdaptor
// on the root object /dev/ulduar/Constellar1
class ObjectManagerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", CONSTELLAR_DBUS_OBJECT_MANAGER_INTERFACE_NAME)

  public:
    explicit ObjectManagerAdaptor(
        ObserverService* service, ObjectTreePublisher* publisher, QObject* parent
    );

    // Registers /activity/current and emits InterfacesAdded
    void activityAppeared(QVariantMapMap const& interfaces);

    // Emits InterfacesRemoved for /activity/current's currently-tracked interfaces
    void activityDisappeared();

  public Q_SLOTS:
    DBusManagerStruct GetManagedObjects();

  Q_SIGNALS:
    void InterfacesAdded(QDBusObjectPath const& object, QVariantMapMap const& interfaces);
    void InterfacesRemoved(QDBusObjectPath const& object, QStringList const& interfaces);

  private:
    ObserverService* m_service;
    ObjectTreePublisher* m_publisher;
    QStringList m_activityInterfaces;  // empty when idle
};
