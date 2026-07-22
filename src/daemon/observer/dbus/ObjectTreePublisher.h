#pragma once

#include <memory>

#include <QDBusConnection>
#include <QObject>
#include <QVariantMap>

class ObserverService;
class ObjectManagerAdaptor;
class ActivityObject;

// Owns the root object (/dev/ulduar/Constellar1)
class ObjectTreePublisher : public QObject
{
    Q_OBJECT

  public:
    explicit ObjectTreePublisher(
        ObserverService* service, QDBusConnection connection, QObject* parent = nullptr
    );

    ~ObjectTreePublisher() override;

  private:
    void onActivityChanged(QVariantMap const& activity);
    void onActivityEnded();

    ObserverService* m_service;
    QDBusConnection m_connection;
    std::unique_ptr<ObjectManagerAdaptor> m_objectManager;
    std::unique_ptr<ActivityObject> m_currentActivity;
};
