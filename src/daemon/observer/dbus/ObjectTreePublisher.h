#pragma once

#include <memory>
#include <optional>
#include <utility>

#include <QDBusConnection>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringView>
#include <QVariantMap>

class ObserverService;
class ObjectManagerAdaptor;
class InterfaceRegistry;
class PropertyTreeObject;

// Owns the root object (/dev/ulduar/Constellar1) and the /activity subtree's
// virtual object.
class ObjectTreePublisher : public QObject
{
    Q_OBJECT

  public:
    // path -> ordered (interface, property-bag) pairs, or nullopt for "no such
    // object". The single source both PropertyTreeObject's Properties/Introspect
    // handling and ObjectManagerAdaptor's GetManagedObjects/InterfacesAdded
    // payloads read from.
    using ObjectSnapshot = QList<std::pair<QString, QVariantMap>>;

    explicit ObjectTreePublisher(
        ObserverService* service, QDBusConnection connection, QObject* parent = nullptr
    );

    ~ObjectTreePublisher() override;

    std::optional<ObjectSnapshot> resolve(QStringView path) const;

  private:
    void onActivityChanged(QVariantMap const& activity);
    void onActivityEnded();

    ObserverService* m_service;
    QDBusConnection m_connection;
    std::unique_ptr<InterfaceRegistry> m_registry;
    std::unique_ptr<ObjectManagerAdaptor> m_objectManager;
    std::unique_ptr<PropertyTreeObject> m_propertyTree;
    bool m_activityLive = false;
};
