#pragma once

#include <memory>

#include <QDBusConnection>
#include <QObject>
#include <QVariantMap>

class ObserverService;
class ObjectManagerAdaptor;
class CurrentActivityObject;

// Owns the root object (/dev/ulduar/Constellar1), its ObjectManager adaptor, and
// the /activity/current child object's registration lifecycle (RFC-006 /
// TASK-009). The persisted collections (/activities, /recordings) are TASK-010.
class ObjectManagerHost : public QObject
{
    Q_OBJECT

  public:
    explicit ObjectManagerHost(
        ObserverService* service, QDBusConnection connection, QObject* parent = nullptr
    );
    ~ObjectManagerHost() override;

  private:
    void onActivityChanged(QVariantMap const& activity);
    void onActivityEnded();

    ObserverService* m_service;
    QDBusConnection m_connection;
    ObjectManagerAdaptor* m_objectManager;
    std::unique_ptr<CurrentActivityObject> m_currentActivity;
};
